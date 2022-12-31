// i'd like to begin by saying that i have no idea what i'm doing


#include<ESP8266WiFi.h>
#include<time.h>
#include<LittleFS.h>

#include "request.h"
#include "splitstr.h"

#include "config.h"

#define  CHUNK_SIZE 256

// response headers
static const char *HEADER_GEM_OK            = "20 text/gemini\r\n";                 // .gmi
static const char *HEADER_MARKDOWN_OK       = "20 text/markdown\r\n";               // .md
static const char *HEADER_PLAIN_OK          = "20 text/plain\r\n";                  // .txt
static const char *HEADER_JPEG_OK           = "20 image/jpeg\r\n";                  // .jpg
static const char *HEADER_BIN_OK            = "20 application/octet-stream\r\n";    // other stuff
static const char *HEADER_NOT_FOUND         = "51 File Not Found\r\n";
static const char *HEADER_INTERNAL_FAIL     = "50 Internal Server Error\r\n";

// tls server
BearSSL::WiFiServerSecure server(PORT);

void setup()
{   
    // setup LittleFS
    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    if(!LittleFS.begin()){
      Serial.println("Error mounting LittleFS!");
      return;
    }

    // setup serial
    Serial.begin(74880);

    // connect to wifi as defined in config.h
    WiFi.mode(WIFI_STA);
    WiFi.hostname(HOSTNAME);
    WiFi.begin(SSID, PASSWD);

    // wait for connection
    while(WiFi.status() != WL_CONNECTED){delay(1000);}
    Serial.println("Connected to WiFi");
    Serial.println("IP address: " + WiFi.localIP().toString());

    // add private cert and key
    BearSSL::X509List *serverCertList = new BearSSL::X509List(server_cert);
    BearSSL::PrivateKey *serverPrivateKey = new BearSSL::PrivateKey(server_private_key);
    
    server.setRSACert(serverCertList, serverPrivateKey);

    // list files in LittleFS
    Dir dir = LittleFS.openDir("/gemini/");
    while (dir.next()){
        Serial.println(dir.fileName());
    }

    // begin accepting connections
    server.begin();

}

void loop()
{
    BearSSL::WiFiClientSecure incomingConnection = server.available();
    if (!incomingConnection){return;} // if no connections then do nothing

    while (incomingConnection.connected()){
        if (incomingConnection.available()){
            String requestString = incomingConnection.readStringUntil('\r\n'); // get request string
            Request request = Request(requestString);

            if (!LittleFS.exists(request.getFile_path())) // if file does not exist
            { 
                if(request.getFile_path().equals("/gemini/status.gmi")) // if request is for status page then generate it on the fly
                {
                    Serial.println("Sending status page");
                    incomingConnection.read();
                    incomingConnection.write((uint8_t*)HEADER_GEM_OK, strlen(HEADER_GEM_OK));
                    incomingConnection.write("# Live status\n");
                    incomingConnection.write(("This page was generated on the fly at " + String(millis()/1000) + " seconds after boot.\n").c_str());
                    incomingConnection.write(("IP address: " + WiFi.localIP().toString() + "\n").c_str());
                    incomingConnection.write(("Free memory: " + String(ESP.getFreeHeap()) + " bytes\n").c_str());
                    incomingConnection.write(("Power voltage: " + String(analogRead(0)) + "V\n").c_str());
                    incomingConnection.write(("Uptime: " + String(millis()/1000) + " seconds\n").c_str());
                    incomingConnection.write(("Connected to: " + String(SSID) + "\n").c_str());
                    incomingConnection.write(("Hostname: " + String(HOSTNAME) + "\n").c_str());
                    incomingConnection.write(("Signal strength: " + String(WiFi.RSSI()) + "dBm\n").c_str());
                    incomingConnection.write("### Files:\n");
                    Dir direct = LittleFS.openDir("/gemini/");
                    while (direct.next()){
                        incomingConnection.write(("=> " + direct.fileName() + " " + direct.fileName()).c_str());
                        incomingConnection.write("\n");
                    }
                    incomingConnection.flush();
                    incomingConnection.stop();
                }
                // Send file not found header
                Serial.println("File not found: " + request.getFile_path());
                incomingConnection.read();
                incomingConnection.write((uint8_t*)HEADER_NOT_FOUND, strlen(HEADER_NOT_FOUND));
                incomingConnection.flush();
                incomingConnection.stop();
            }
            else // file exists
            {
                File file = LittleFS.open(request.getFile_path().c_str(), "r"); // open file

                if (!file.isFile()) // error opening file
                {
                    file.close();
                    Serial.println("Error opening file: " + request.getFile_path());

                    incomingConnection.read();
                    incomingConnection.write((uint8_t*)HEADER_INTERNAL_FAIL, strlen(HEADER_INTERNAL_FAIL));
                    incomingConnection.flush();
                    incomingConnection.stop();
                }
                else // file is OK
                {
                    Serial.println("Sending file: " + request.getFile_path());
                    incomingConnection.read();  // read the rest of the incomig data
                                                
                    String extension = request.getFile_ext(); // get the file extension and send the required header
                    if (extension.equals("gmi"))        {incomingConnection.write((uint8_t*)HEADER_GEM_OK, strlen(HEADER_GEM_OK));} 
                    else if (extension.equals("txt"))   {incomingConnection.write((uint8_t*)HEADER_PLAIN_OK, strlen(HEADER_PLAIN_OK));}
                    else if (extension.equals("md"))    {incomingConnection.write((uint8_t*)HEADER_MARKDOWN_OK, strlen(HEADER_MARKDOWN_OK));} 
                    else if (extension.equals("jpg"))   {incomingConnection.write((uint8_t*)HEADER_JPEG_OK, strlen(HEADER_JPEG_OK));} 
                    else                                {incomingConnection.write((uint8_t*)HEADER_BIN_OK, strlen(HEADER_BIN_OK));}
                    
                    // time to read the file itself

                    // calculate count of chunks and size of the stuff that didn't fit
                    int fsize = file.size();
                    int chunk_count = (fsize / CHUNK_SIZE);
                    int rest_size = (fsize % CHUNK_SIZE);

                    // read chunks from the file and send them
                    char chunk[CHUNK_SIZE];
                    for (int i = 0; i < chunk_count; i++){
                        file.read((uint8_t*)chunk, CHUNK_SIZE);
                        incomingConnection.write((uint8_t*)chunk, CHUNK_SIZE);
                    }

                    // read and send what's left
                    char rest[rest_size];
                    file.read((uint8_t*)rest, rest_size);
                    incomingConnection.write((uint8_t*)rest, rest_size);

                    file.close(); // guess what this one does ^_^

                    // those ones too
                    incomingConnection.flush();
                    incomingConnection.stop();
                }
            }
        }
    }
}

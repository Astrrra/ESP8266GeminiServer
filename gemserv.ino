// i'd like to begin by saying that i have no idea what i'm doing


#include<ESP8266WiFi.h>
#include<time.h>
#include<LittleFS.h>

#include "request.h"
#include "splitstr.h"

#include "config.h"

#define	CHUNK_SIZE 256

// response headers
static const char *HEADER_GEM_OK 			= "20 text/gemini\r\n";								// .gmi
static const char *HEADER_MARKDOWN_OK	    = "20 text/markdown\r\n";							// .md
static const char *HEADER_PLAIN_OK 		    = "20 text/plain\r\n";								// .txt
static const char *HEADER_JPEG_OK			= "20 image/jpeg\r\n";								// .jpg
static const char *HEADER_BIN_OK			= "20 application/octet-stream\r\n";	            // other stuff
static const char *HEADER_NOT_FOUND 		= "51 File Not Found\r\n";
static const char *HEADER_INTERNAL_FAIL 	= "50 Internal Server Error\r\n";

// tls server
BearSSL::WiFiServerSecure server(PORT);

void setup()
{   
    // setup LittleFS
    LittleFSConfig cfg;
    cfg.setAutoFormat(false);

    if (!LittleFS.begin()) // restart if LittleFS failed
    { 
        delay(1000);
        ESP.restart();
    }

    // connect to wifi as defined in config.h
    WiFi.mode(WIFI_STA);
    WiFi.hostname(HOSTNAME);
    WiFi.begin(SSID, PASSWD);
    // wait for connection
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
    }

    // add private cert and key
    BearSSL::X509List *serverCertList = new BearSSL::X509List(server_cert);
    BearSSL::PrivateKey *serverPrivateKey = new BearSSL::PrivateKey(server_private_key);
    
    server.setRSACert(serverCertList, serverPrivateKey);

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
                // Send file not found header
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

                    incomingConnection.read();
                    incomingConnection.write((uint8_t*)HEADER_INTERNAL_FAIL, strlen(HEADER_INTERNAL_FAIL));
                    incomingConnection.flush();
                    incomingConnection.stop();
                }
                else // file is OK
                {
                    incomingConnection.read();  // read the rest of the incomig data
                                                
                    String e = request.getFile_ext(); // get the file extension and send the required header
                    if (e.equals("gmi"))
                    {
                            Serial.write(HEADER_GEM_OK, strlen(HEADER_GEM_OK));
                            incomingConnection.write((uint8_t*)HEADER_GEM_OK, strlen(HEADER_GEM_OK));
                    } 
                    else if (e.equals("txt")) 
                    {
                            Serial.write(HEADER_PLAIN_OK, strlen(HEADER_PLAIN_OK));
                            incomingConnection.write((uint8_t*)HEADER_PLAIN_OK, strlen(HEADER_PLAIN_OK));
                    }
                    else if (e.equals("md")) 
                    {
                            Serial.write(HEADER_MARKDOWN_OK, strlen(HEADER_MARKDOWN_OK));
                            incomingConnection.write((uint8_t*)HEADER_MARKDOWN_OK, strlen(HEADER_MARKDOWN_OK));
                    } 
                    else if (e.equals("jpg")) 
                    {
                            Serial.write(HEADER_JPEG_OK, strlen(HEADER_JPEG_OK));
                            incomingConnection.write((uint8_t*)HEADER_JPEG_OK, strlen(HEADER_JPEG_OK));
                    } 
                    else 
                    {
                            Serial.write(HEADER_BIN_OK, strlen(HEADER_BIN_OK));
                            incomingConnection.write((uint8_t*)HEADER_BIN_OK, strlen(HEADER_BIN_OK));
                    }
                    
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

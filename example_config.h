#ifndef _CONFIG_H_
#define _CONFIG_H_

#define SSID   "wifi_name"
#define PASSWD  "wifi_password"

#define HOSTNAME  "gemini"
#define PORT      1965

const char server_private_key[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
your key here
-----END RSA PRIVATE KEY-----

)EOF";

// The server's public certificate which must be shared
const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
your certificate here
-----END CERTIFICATE-----


)EOF";

#endif // _CONFIG_H_

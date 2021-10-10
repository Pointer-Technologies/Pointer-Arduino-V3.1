/*
 * Authored on Arduino 1.8.13, in C++
 * Antonios Antoniou
 * ************************************************************
 * Kind of the final form of the Pointer's function code. 
 * Makes the soft Access Point detectable and uses it to get the  
 * WiFi credentials, in order to establish a connection. 
 * The ESP8266 then uses that connection and the local IP in order 
 * to be controlled by any device that knows the IP.
 * ************************************************************
 * Revisited on 3-11-2020
 * This version ditches the EEPROM functionality.
*/

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <String.h>

// Define the vibration motor and crystal pins.
#define VIBRATION_OUTPUT 5
#define SOUND_OUTPUT 4


// The module SSID and (blank) password.
// The placeholders for the new network credentials.
const char* ESP_SSID = "PointerModuleAP";
const char* ESP_PASS = "";
String ssid = "";
String password = "";

// Set the UDP IP, port and package sender.
const char* UDP_IP = "192.168.43.200"; 
unsigned int UDP_PORT = 4210;
WiFiUDP udp;

// Standard IP values. Makes the app's job easier.
//IPAddress IP_LOCAL (192, 168, 1, 10);
//IPAddress GATEWAY (192, 168, 1, 1);
//IPAddress SUBNET (255, 255, 0, 0); 

// ------------ WEB SERVER ------------ //
WiFiServer server(80); // Initalize the server in port 80.
String header; // HTTP header.
String vibrationState = "off"; // The vibration motor power state.
String soundState = "off"; // The piezoelectric crystal power state.

// Initalize the timeout margins.
unsigned long currentTime = millis();
unsigned long initialTime = 0;
unsigned long timeout = 2000;
unsigned long connectionTimeout = 5000;
// -------------------------------------- //


// ------------ ACCESS POINT ------------ //
//  HTML string to be used for the Access Point webpage.
//  The first two forms will contain the SSID and password.

//--!(Obsolete)!
//  The third form contains the number of modules that have been set up before in the same network.
//  This will take care of avoiding colliding local IP addresses.
String html = "<!DOCTYPE html> "
"<html>"
    "<head>"
        "<script type=\"text/javascript\">"
            "function invokeNative(){"
                "MessageInvoker.postMessage('Network info has been submitted.')"
            "}"
        "</script>"
        "<content=\"width=device-width, initial-scale=1\">"
    "</head>"
"<title>Pointer setup</title>"
"<body>"
    "<h1 style=\"text-align: center; font-family: Calibri\">Please enter your network's credentials</h1>"
    "<div class=\"form\" style=\"margin: auto; padding: 10px;\">"
        "<form>"
            "<label for=\"SSID\" style=\"font-family: Calibri; margin: auto; width: 50%\">SSID:</label>"
            "<input type=\"text\" id=\"SSID\" name=\"SSID\"\"><br><br>"
            "<label for=\"password\" style=\"font-family: Calibri; margin: auto; width: 50%\">Password:</label>"
            "<input type=\"text\" id=\"password\" name=\"password\"\"><br><br>"
//            "<label for=\"devicesNum\" style=\"font-family: Calibri; color: #F60002; margin: auto; width: 50%\">How many more modules have been connected to the same network?</label>"
//            "<input type=\"number\" id=\"devicesNum\" name=\"devicesNum\" min=\"0\" max=\"20\"><br><br>"
            "<input type=\"submit\" value=\"Submit\" onclick=\"invokeNative()\">"
        "</form>"
    "</div>"
    "<h3 style=\"color: #F60002; font-family: Calibri\">"
    "Clicking the \"Submit\" button will send the credentials to the Pointer module. <br>Please make sure the two forms are right.</h3>"
"</body>"
"</html>";

// Test function for the ESP server.
void serveAdmin(ESP8266WebServer *webServer) {
    String message;  
      
    // Create a string containing all the arguments, send them out to the serial port
    Serial.println(webServer->arg("SSID"));
    Serial.println(webServer->arg("password"));

    // Construct a message to tell the user that the change worked
    message = "New settings will take effect after restart";     

    // Reply with a web page to indicate success or failure
    message = "<html><head><meta http-equiv='refresh' content='5;url=/' /></head><body>" + message;
    message += "<br/>Redirecting in 5 seconds...</body></html>";
    webServer->sendHeader("Content-Length", String(message.length()));
    webServer->send(200, "text/html", message);
}

// Initialize the server and set the function prototype.
ESP8266WebServer ESPserver(80);
void handleRoot(ESP8266WebServer *ESPserver, String *ssid, String *password){
    // Create an HTML website once a user connects to the AP IP. 
    ESPserver->send(200, "text/html", html);

    // Then store the credentials.
    *ssid = ESPserver->arg("SSID");
    *password = ESPserver->arg("password");
}
// ----------------------------------------- //

// Turns the IP Address into a usable string.
String ipToString(const IPAddress &ipAddress){
  return String(ipAddress[0]) + String(".") +
  String(ipAddress[1]) + String(".") +
  String(ipAddress[2]) + String(".") +
  String(ipAddress[3]);
}

void setup() {
    // Initialize serial connection.
    Serial.begin(115200);
    Serial.println("Setting up the softAP");

    // Initialize the sound and vibration pins as outputs.
    pinMode(VIBRATION_OUTPUT, OUTPUT);
    pinMode(SOUND_OUTPUT, OUTPUT);
    
    // Make sure the two outputs are initially off.
    digitalWrite(VIBRATION_OUTPUT, LOW); 
    digitalWrite(SOUND_OUTPUT, LOW);

    // Start the Access Point and get the ESP IP Address.
    WiFi.softAP(ESP_SSID, ESP_PASS);
    IPAddress APIP = WiFi.softAPIP();
    Serial.print("Soft Access Point: ");
    Serial.println(APIP);

    // Begin the ESP server.
    // Bind the handleRoot() function with the server, ssid and password placeholders.
    ESPserver.on("/", std::bind(handleRoot,&ESPserver, &ssid, &password));
    ESPserver.begin();
    Serial.println("Server has started.");

    // Get the ESP server ready for clients, 
    // while the module isn't connected to a WiFi.
    while((ssid == "" && password == "") || WiFi.waitForConnectResult() == WL_NO_SSID_AVAIL ){
        ESPserver.handleClient();
    }

    // Stop the local server, disconnect the Access Point and change WiFi modes.
    // Essentialy gets the ESP ready to listen for requests from the Internet.
    ESPserver.stop();
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(500); // Delay to make sure the inputs are saved and ready to use.

    WiFi.begin(ssid, password);
    
    // Signify the attempt to connect after getting the credentials.
    while(WiFi.status() != WL_CONNECTED){
        Serial.println("Attempting to connect...");
        delay(2000);   
    }

    // Wait a second so we can make sure we'll connect and get the IP address.
     delay(1000);
     IPAddress LOCAL_IP = WiFi.localIP();
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.psk());
    Serial.println(WiFi.localIP());

    // Initialize a buffer and fill it with the IP Address,
    // that is turned to a string and sent via UDP.
    char buffer[20];
    String LOCAL_IP_STRING = ipToString(LOCAL_IP);
    LOCAL_IP_STRING.toCharArray(buffer, 20);
    udp.beginPacket(UDP_IP, UDP_PORT);
    udp.write(buffer, LOCAL_IP_STRING.length());
    udp.endPacket();        

    Serial.print("Sent local IP by UDP: ");
    Serial.println(LOCAL_IP_STRING);
    
    // Start the WiFi server.
    server.begin(); 
}

void loop() {
    WiFiClient client = server.available(); // Listen for clients.

    if (client) {
        // Detect a new client, initialize the start time
        // and process a request, if any, within the timeout margin.
        Serial.println("Welcome to the PointerProject configuration.");
        String currentLine = "";
        currentTime = millis();
        initialTime = currentTime;
    
        while(client.connected() && (currentTime - initialTime <= timeout)){
            currentTime = millis();
            if(client.available()){
                char req = client.read();
                Serial.write(req);
                header += req;
                
                if(req == '\n'){
                    if(currentLine.length() == 0){
                        /*
                         * If the current line is blank, the 2 'new line' characters signify the end of an HTTP request.
                         * So we follow up with a response.
                         * The HTTP header needs to start with a status code (at least for now, for debugging purposes).
                         * The (HTTP) client needs to know what's coming, hence the 'content-type'.
                         * We then close the information package with a blank line.
                         */
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
    
                        // Turn the motors on or off.
                        if(header.indexOf("GET /5/on") >= 0){
                            Serial.println("Vibration is currently on.");
                            vibrationState = "on";
                            digitalWrite(VIBRATION_OUTPUT, HIGH);  
                        }
                        else if(header.indexOf("GET /5/off") >= 0){
                            Serial.println("Vibration is currenly off.");
                            vibrationState = "off";
                            digitalWrite(VIBRATION_OUTPUT, LOW);
                        }
                        else if(header.indexOf("GET /4/on") >= 0){
                            Serial.println("Sound is currently on");
                            soundState = "on";
                            digitalWrite(SOUND_OUTPUT, HIGH);
                        }
                        else if(header.indexOf("GET /4/off") >= 0){
                            Serial.println("Sound is currently off");
                            soundState = "off";
                            digitalWrite(SOUND_OUTPUT, LOW);
                        }
    
                        // Lastly, we need to display the HTML page:
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons.
                        client.println("<style>html { font-family: Calibri; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".button { background-color: #0099FF; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                        client.println(".button2 {background-color: #AB0501;}</style></head>");
                        // Web Page Heading:
                        client.println("<body><h1>ESP8266 PointerProject Configuration Server</h1>");
                        
                        // Display current state, and ON/OFF buttons for the vibration motor.
                        client.println("<p>Sound state: " + soundState + "</p>");
                        // If the outputState is off, it displays the ON button and vice versa.
                        if (soundState == "off") {
                            client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
                        } 
                        else if (soundState == "on") {
                             client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
                        } 
    
                        // Now, display the state of the sound crystal.
                        client.println("<p>Vibration state: " + vibrationState + "</p>");
                        if(vibrationState == "off"){
                            client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>"); 
                        }
                        else if (vibrationState == "on"){
                            client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }
                        client.println("</body></html>"); // The HTML format is over here.
    
                        // The response has to end with  a blank line again.
                        client.println();
                        break; // Breaks out of the 'while' loop.
                    }
                    else{
                        // This 'else' statement refers to the req variable being a new line.
                        // Clears the current line:
                        currentLine = "";
                    }
                }
                else if(req != '\r'){
                    // Refers to req not being a newline, AND NOT a return character.
                    // So we add req to the end of the current line:
                    currentLine += req;
                }
            } 
        }
         header = ""; // Clear the header. 
         client.stop(); // Connection is closed.
         Serial.println("Connection was terminated.");
         Serial.println();
    }

        if(WiFi.waitForConnectResult() == WL_NO_SSID_AVAIL){
            ESP.restart();
    }
}

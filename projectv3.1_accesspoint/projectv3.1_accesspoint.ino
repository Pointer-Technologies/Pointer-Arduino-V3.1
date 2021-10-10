#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <String.h>

// The module SSID and (blank) password.
// The placeholders for the new network credentials.
const char* ESP_SSID = "PointerModuleAP";
const char* ESP_PASS = "";
String ssid = "";
String password = ""; 

// HTML string to be used for the Access Point webpage.
String html ="<!DOCTYPE html> <html><head><content=\"width=device-width, initial-scale=1\">"
"<title>Pointer setup</title> <body> <h1 style=\"text-align: center; font-family: Calibri\">Please enter your network's credentials</h1>"
"<div class=\"form\" style=\"margin: auto; padding: 10px;\"> <form> <label for=\"SSID\" style=\"font-family: Calibri; margin: auto; width: 50%\">SSID:</label> <input type=\"text\" id=\"SSID\" name=\"SSID\""
"\"><br><br> <label for=\"password\" style=\"font-family: Calibri\">Password:</label> <input type=\"text\" id=\"password\" name=\"password\""
"\"><br><br> <input type=\"submit\" value=\"Submit\" onclick=\"alert('Network information has been submitted')\"></form> </div>"
"<h3 style=\"color: #F60002; font-family: Calibri\">"
"Clicking the \"Submit\" button will send the credentials to the Pointer module. <br>Please make sure the two forms are right.</h3></body></html>";

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
 void handleRoot(ESP8266WebServer *ESPserver){
    // Create an HTML website once a user connects to the local IP. 
    ESPserver->send(200, "text/html", html);

    // Create a string containing all the arguments, send them out to the serial port.
    Serial.println("The SSID probably is: " + ESPserver->arg("SSID"));
    Serial.println(ESPserver->arg("password"));
    // Then store the credentials.
    ssid = ESPserver->arg("SSID");
    password = ESPserver->arg("password");
 }

void setup() {
    Serial.begin(115200);
    Serial.println("Setting up the softAP");

    // Set to Access Point mode and print the local IP.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ESP_SSID, ESP_PASS);
    IPAddress APIP = WiFi.softAPIP();
    Serial.print("Soft Access Point: ");
    Serial.println(APIP);

    // Begin the ESP server.
    ESPserver.on("/", std::bind(handleRoot,&ESPserver));
    ESPserver.begin();
    Serial.println("Server has started.");
}

void loop() {
    // Get the server ready for clients.
    ESPserver.handleClient();

    // Print the credentials. Make sure everything's good. (is it tho?)
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
}

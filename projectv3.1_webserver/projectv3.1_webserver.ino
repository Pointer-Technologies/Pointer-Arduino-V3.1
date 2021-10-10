#include <ESP8266WiFi.h>

// Assign output variables to GPIO pins
#define SOUND_OUTPUT 4
#define VIBRATION_OUTPUT 5

const char* ssid     = "COSMOTE-291343";
const char* password = "eKN9uuUtvteQ4rPr";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String soundState = "off";
String vibrationState = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long initialTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeout = 2000;

void setup() {
  Serial.begin(115200);
  
  // Initialize the output variables as outputs
  pinMode(VIBRATION_OUTPUT, OUTPUT);
  pinMode(SOUND_OUTPUT, OUTPUT);
  
  // Set outputs to LOW
  digitalWrite(VIBRATION_OUTPUT, LOW);
  digitalWrite(SOUND_OUTPUT, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();

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
                        /*If the current line is blank, the 2 'new line' characters signify the end of an HTTP request.
                         * So we follow up with a response.
                         * The HTTP header needs to start with a status code (at least for now, for debugging purposes).
                         * The (HTTP) client needs to know what's coming, hence the 'content-type'.
                         * We then close the information package with a blank line.*/
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
    
                        // Turn the motor on or off.
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
}


// Load Wi-Fi library
#include <WiFi.h>
#include <GyverOLED.h>

GyverOLED<SSH1106_128x64> oled;

//smile
int dotsx[] = {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96};
int dotsy[] = {45, 45, 46, 46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 47, 46, 46, 46, 45, 45};

const int DRYNESS_THRESHOLD = 600;

bool happy = false;
// frown

// Replace with your network credentials
const char* ssid = "Jane Huang";
const char* password = "61Carnival";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variable to store onboard LED state
String picoLEDState = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {

  // Start Serial Monitor
  Serial.begin(115200);

  // Initialize the LED as an output
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT_PULLUP);

  // Set LED off
  digitalWrite(LED_BUILTIN, LOW);

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);

  // Display progress on Serial monitor
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.print("WiFi connected at IP Address ");
  Serial.println(WiFi.localIP());

  oled.init();  
  
  oled.clear();   
  oled.update(); 

  
  // Start Server
  server.begin();
  
  happy = analogRead(A0) < DRYNESS_THRESHOLD;


}

int old2 = 0;
int old1 = 0;
int moisture = 0;

void loop() {

  WiFiClient client = server.available();   // Listen for incoming clients

  oled.clear();
  oled.setCursorXY(28, 26);

  old2 = old1;
  old1 = moisture;
  moisture = analogRead(A0);

  //Serial.println("Current: " , moisture , " Previous: ", old1 , " Older: " , old2);
  Serial.print("Current: ");
  Serial.print(moisture);
  Serial.print(" Previous: ");
  Serial.print(old1);
  Serial.print(" Old2: ");
  Serial.println(old2);

  //getting dryer
  if (happy == true && moisture > DRYNESS_THRESHOLD && old1 > DRYNESS_THRESHOLD && old2 > DRYNESS_THRESHOLD)
  {
    happy = false;
    Serial.println("Dryness threshold reached");
  }

  // got watered
  if (moisture < DRYNESS_THRESHOLD && abs(moisture-old2) > 30 )
  {
    //watered animation?
    Serial.print(abs(moisture-old2));
    Serial.println("Plant was watered");
    happy = true;
  }

  

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Switch the LED on and off
            if (header.indexOf("GET /led/on") >= 0) {
              Serial.println("LED on");
              picoLEDState = "on";      
              digitalWrite(LED_BUILTIN, HIGH);
            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("LED off");
              picoLEDState = "off";
              digitalWrite(LED_BUILTIN, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // CSS to style the on/off buttons
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #F23A3A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Pico W Awake Control</h1>");

            // Display current state, and ON/OFF buttons for Onboard LED
            if (picoLEDState == "on")
            {
              client.println("<p>PICO is Awake</p>");

            }
            else
            {
              client.println("<p>PICO is Asleep</p>");
            }
            
            // Set buttons
            if (picoLEDState == "off") {
              
              //picoLEDState is off, display the ON button
              client.println("<p><a href=\"/led/on\"><button class=\"button\">Wake Up!</button></a></p>");
            } else {

              //picoLEDState is on, display the OFF button
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">Go to Sleep!</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");

    //oled.print("LED " + picoLEDState);
  } 
  
  if (picoLEDState == "on") 
  {
    //oled.print(WiFi.localIP());
    oled.circle(32, 20, 6, OLED_FILL);
    oled.circle(96, 20, 6, OLED_FILL);
    oled.circle(32, 20, 15, OLED_STROKE);
    oled.circle(96, 20, 15, OLED_STROKE);

    if (happy)
    {
      for (int i = 0; i < 65; i ++)
      {
        oled.dot(dotsx[i], dotsy[i]+4, 1);
      }   
    }
    else
    {
      for (int i = 0; i < 65; i ++)
      {
        oled.dot(dotsx[i], (-1*(dotsy[i]-47))+47, 1);
      }
    }
  }
  else
  {
    //oled.print(WiFi.localIP());
    //oled.line(17, 20, 47, 20, OLED_STROKE);\
    //oled.line(81, 20, 111, 20, OLED_STROKE); 

  }
  
  oled.update();

}

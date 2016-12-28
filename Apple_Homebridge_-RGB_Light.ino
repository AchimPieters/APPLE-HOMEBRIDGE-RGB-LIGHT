/* Project name: APPLE HOMEBRIDGE – RGB LIGHT 
*  Project URI: http://www.studiopieters.nl/apple-homebridge-rgb-light/ 
*  Description: APPLE HOMEBRIDGE – RGB LIGHT
*  Version: APPLE HOMEBRIDGE – RGB LIGHT 2.0.0 
*  License: GNU General Public License V2 or later  */

#include <ESP8266WiFi.h>
#include <math.h>



#define redPin 13 //D7 - Red channel
#define grnPin 12 //D6 - Green channel
#define bluPin 14 //D5 - Blue channel

WiFiServer server(80); //Set server port

String readString;           //String to hold incoming request
String hexString = "000000"; //Define inititial color here (hex value)

int state;

int r;
int g;
int b;

float R;
float G;
float B;

int x;
int V;



///// WiFi SETTINGS - Replace with your values /////////////////
const char* ssid = "YOUR_ROUTER_SSID";
const char* password = "YOUR_ROUTER_PASSWORD";
IPAddress ip(192,168,1,10);      // set a fixed IP for the NodeMCU
IPAddress gateway(192,168,1,1);  // Your router IP
IPAddress subnet(255,255,255,0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("_");
  }
  Serial.println();
  Serial.println("Done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  server.begin();                   
}

void allOff() {
  state = 0;
  analogWrite(redPin, 0);
  analogWrite(grnPin, 0);
  analogWrite(bluPin, 0);
}

//Write requested hex-color to the pins
void setHex() {
  state = 1;
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  analogWrite(redPin, (r));
  analogWrite(grnPin, (g));
  analogWrite(bluPin, (b));
}

//Compute current brightness value
void getV() {
  R = roundf(r/2.55);
  G = roundf(g/2.55);
  B = roundf(b/2.55);
  x = _max(R,G);
  V = _max(x, B);
}

//For serial debugging only
void showValues() {
  Serial.print("Status on/off: ");
  Serial.println(state);
  Serial.print("RGB color: ");
  Serial.print(r);
  Serial.print(".");
  Serial.print(g);
  Serial.print(".");
  Serial.println(b);
  Serial.print("Hex color: ");
  Serial.println(hexString);
  getV();
  Serial.print("Brightness: ");
  Serial.println(V);
  Serial.println("");
}

void setup(){
  Serial.begin(9600);
  setHex(); //Set initial color after booting. Value defined above
  WiFi.mode(WIFI_STA);
  WiFiStart();
  //showValues(); //Uncomment for serial output
}

void loop() {
  //Reconnect on lost WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    WiFiStart();
  }
 
  WiFiClient client = server.available();
 
  if (!client) {
    return;
  }
 
  while(client.connected() && !client.available()) {
    delay(1);
  }
   
  //Respond on certain Homebridge HTTP requests
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n') {
          //Serial.print("Request: "); //Uncomment for serial output
          //Serial.println(readString); //Uncomment for serial output
         
          //Send reponse
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
         
          //On
          if(readString.indexOf("on") >0) {
            setHex();
            //showValues();
          }
     
          //Off
          if(readString.indexOf("off") >0) {
            allOff();
            //showValues();
          }

          //Set color
          if(readString.indexOf("set") >0) {
            hexString = "";
            hexString = (readString.substring(9,15));
            setHex();
            //showValues();
          }
         
          //Status on/off
          if(readString.indexOf("status") >0) {
          client.println(state);
          }

          //Status color (hex)
          if(readString.indexOf("color") >0) {
          client.println(hexString);
          }

          //Status brightness (%)
          if(readString.indexOf("bright") >0) {
          getV();
          client.println(V);
          }
         
          delay(1);
          client.stop();
          readString="";
        }
      }
    }
  }
}

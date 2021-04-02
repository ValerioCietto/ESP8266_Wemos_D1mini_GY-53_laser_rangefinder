#include <SoftwareSerial.h>
#include <Arduino.h>
//#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>



void handleRoot();
void handleDati();
void muovi();
void message();
void messageFast();
void serverSetup();
void receiveOnly();
int wifiSetup();
int readDistance1();

SoftwareSerial swSer(D2, D3, false, 256);
/* Set these to your desired credentials. */
const char *ssid = "Wifi";
const char *password = "wifidicasa1.";

boolean mute = false;
boolean small_laser=true;
boolean big_laser=false;

ESP8266WebServer server(80);

MDNSResponder mdns;
String serialMessage="";
int data=0;


void setup(){

  Serial.begin(115200);
  serverSetup();
  wifiSetup();

  if(small_laser){
    swSer.begin(9600);
    swSer.write(0XA5);
    swSer.write(0X45);    // Enable Output
    swSer.write(0XEA);

  }
  else if(big_laser){
    swSer.begin(19200);
    swSer.println("D"); 
  }
}

void loop(){
//  Serial.println(counter);
    if(Serial.available()){
      serialMessage = Serial.readString();
    }
    server.handleClient();
  if(millis()%100==0 && small_laser){
    int distance = readDistance1();
    
    if(distance>0 && distance<2500){
      if(!mute){
        Serial.println(distance);
      }
      data=distance;
    }
  }
  if(millis()%3000==0 && big_laser){
    String distance = readBigLaserDistance();
    float distance_float = (distance.substring(2,7)).toFloat();
    data = (int)(distance_float*1000);
    
  }
}




int readDistance1(){
  unsigned char re_Buf[11], counter = 0;
  unsigned char sign = 0;
  byte data_Buf[3] = {0};
  unsigned char i = 0, sum = 0;
  uint16_t distance = 0;
  while (swSer.available()) {
    re_Buf[counter] = (unsigned char)swSer.read();
    //if (counter == 0 && re_Buf[0] != 0x5A) return;
    counter++;
    if (counter == 8){
      counter = 0;               // Get Ready for the next stream
      sign = 1;
    }
  }
  if (sign){
    sign = 0;
    for (i = 0; i < 7; i++)
      sum += re_Buf[i];
    if (sum == re_Buf[i] ) {
      data_Buf[0] = re_Buf[4];
      data_Buf[1] = re_Buf[5];
      data_Buf[2] = re_Buf[6];
      distance = data_Buf[0] << 8 | data_Buf[1];
    }
  }
  return distance;

}

String readBigLaserDistance(){
  String buffer = "";
  swSer.println("D"); // use 'M' to do slow accurate measurement, 'D' for fast mesurements
  while (swSer.available()){
    char c = swSer.read();
    buffer= buffer+c;
    if(c=='\n'||c=='\0'||c=='\r'){
      
    }
    //Serial.println(buffer);
  }
  Serial.println(buffer);

  return buffer;
}

void serverSetup(){
  server.on("/muovi", muovi);
  server.on("/dati", handleDati);
  server.on("/message", message);
  server.on("/mx", messageFast);
  server.on("/rx", receiveOnly);
  server.on("/", handleRoot);

  server.begin();
  if(!mute){
    Serial.println("HTTP server started");
  }

}



int wifiSetup(){

  int n = WiFi.scanNetworks();
  if(!mute){
    Serial.println(n);
    Serial.println("ricerca SSID named wifi");
  }
  boolean connectedToKnown =false;
  for (int i =0; i<n; i++){
    if(!mute){
      Serial.println(i);
      Serial.println(WiFi.SSID(i));
    }
    if(WiFi.SSID(i) == "Wifi1"){

      WiFi.begin("Wifi1", "wifidicasa1.");

      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if(!mute){
          Serial.print(".");
        }
      }
      if(!mute){
        Serial.println("");
        Serial.println("WiFi connected");

        Serial.println("Server started");

        // Print the IP address
        Serial.println(WiFi.localIP());
      }
      connectedToKnown =true;
    }
    else if(WiFi.SSID(i) == "FASTWEB-ICPJ4F"){
      /*
       *Vodafone-34287319
       *458le4m54w7euyw 
       */
      WiFi.begin("D-Link-CD2743", "sstagz9y4ctxmjer");

      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if(!mute){
          Serial.print(".");
        }
      }
      if(!mute){
        Serial.println("");
        Serial.println("WiFi connected");

        Serial.println("Server started");

        // Print the IP address
        Serial.println(WiFi.localIP());
      }
      connectedToKnown =true;
    }
    else{

    }
  }
  if(!connectedToKnown){
    if(!mute){
      Serial.println("init soft AP");
    }
    WiFi.softAP(ssid, password);
    //while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    delay(5000);
    if(!mute){
      Serial.println("done");
    }
    IPAddress myIP = WiFi.softAPIP();
    if(!mute){
      Serial.print("AP IP address: ");
      Serial.println(myIP);

      Serial.println("Server started");
    }
  }
  if (!mdns.begin("laser1", WiFi.localIP())) {
    if(!mute){     //example address http://esp8266.local/
      Serial.println("Error setting up MDNS responder!");
    }
  }else{
    if(!mute){
      Serial.println("mDNS responder started");
    }
  }
  if(!mute){
    Serial.println(" ");
  }



}
void message(){
  int startMillis=millis();
  String response = "messaggio";
  String receivedMessage = server.arg("message");
  Serial.println(receivedMessage);

  response= response+" "+receivedMessage+" "+serialMessage;
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.send(200, "text/html", response);
  int stopMillis= startMillis-millis();
  Serial.println(stopMillis);

}
void messageFast(){
  int startMillis=millis();
  String response = "messaggio";
  String receivedMessage = server.arg("mx");
  Serial.println(receivedMessage);
  response= response+" "+receivedMessage+" "+serialMessage;
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.send(200, "text/html", response);

  int stopMillis= startMillis-millis();
  Serial.println(stopMillis);
}
void receiveOnly(){
  int startMillis=millis();
  String response = "";
  String receivedMessage = server.arg("mx");
  //Serial.println(receivedMessage);
  response= response+" "+serialMessage;
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.send(200, "text/html", response);

  //Serial.println(stopMillis);
}
void muovi(){
  String response = "Muovi ricevuto:)";
  server.send(200, "text/html", response);
  String asse = server.arg("asse");
  String distanza = server.arg("distanza");
  Serial.println("3");//muovi
  delay(1000);
  Serial.println("4");//ferma
}

void handleDati(){
  String response = "";
  response= response+data;
  Serial.print(data);
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.send(200, "text/html", response);
}

void handleRoot() {


  char text[2000];
  int seconds= millis()/1000%60;
  int minutes= millis()/1000/60%60;
  int hours  = millis()/1000/60/60;
  //int day = hours%365;

  sprintf(text, " <meta http-equiv=\"refresh\" content=\"5\" />");
  sprintf(text+strlen(text),"Online from %d:%d:%d, distanza=%d", hours,minutes,seconds,data);
  
  if(!mute){
    Serial.println("diagnostica fornita.");
  }
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    
  server.send(200, "text/html", text);
}

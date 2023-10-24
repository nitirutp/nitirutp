#define BLYNK_TEMPLATE_ID     "TMPL64LWRrm_i"
#define BLYNK_TEMPLATE_NAME   "Quickstart Template"
#define BLYNK_AUTH_TOKEN      "-j9a24gdA-YLBBdq0pXv3EcdEmVBEKo8"
#define LINE_TOKEN            "PXb9Do7MdNCZPuhMuiZWeleWnY26hnLxw0L4HplxEkP"
#define BLYNK_PRINT Serial
#define DHTPIN 15
#define DHTTYPE DHT22  // DHT 22 (AM2302), AM2321

#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <DHT.h>


char ssid[] = "Kankorlub_2.4G";
char pass[] = "Nitirut42";

const char* serverAddress = "http://34.124.201.103/insert.php";
//const char* serverAddress = "http://192.168.30.252/arduino_project_npru/insert.php";

int pm1;
int pm2_5;
int pm10;
int humidity;
int temperature;

SoftwareSerial mySerial(16, 17); // RX, TX
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println(LINE.getVersion());
}

void loop() {
  //Blynk.run();
  PM();
  DHT();
   
  if(WiFi.status() != WL_CONNECTED) { 
    connectWiFi();
  }

  String postData = "&pm10=" + String(pm10) + "&pm2_5=" + String(pm2_5) + "&pm1=" + String(pm1) + "&temperature=" + String(temperature) + "&humidity=" + String(humidity);  

  HTTPClient http; 
  http.begin(serverAddress);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
  
  int httpCode = http.POST(postData); 
  String payload = http.getString(); 
  
  if(httpCode > 0) {
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();  //Close connection
  
  Serial.print("URL : "); Serial.println(serverAddress); 
  Serial.print("Data: "); Serial.println(postData); 
  Serial.print("httpCode: "); Serial.println(httpCode); 
  Serial.print("payload : "); Serial.println(payload); 
  Serial.println("--------------------------------------------------");
 
  Line(pm10, pm2_5, pm1, temperature, humidity);
  
}

void PM() {
  int index = 0;
  char value;
  char previousValue;

  while (mySerial.available()) {
    value = mySerial.read();
    if ((index == 0 && value != 0x42) || (index == 1 && value != 0x4d)) {
      Serial.println("Cannot find the data header.");
      break;
    }

    if (index == 4 || index == 6 || index == 8 || index == 10 || index == 12 || index == 14) {
      previousValue = value;
    } else if (index == 5) {
      pm1 = 256 * previousValue + value;
      Serial.print("{ ");
      Serial.print("\"pm1\": ");
      Serial.print(pm1);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 7) {
      pm2_5 = 256 * previousValue + value;
      Serial.print("\"pm2_5\": ");
      Serial.print(pm2_5);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 9) {
      pm10 = 256 * previousValue + value;
      Serial.print("\"pm10\": ");
      Serial.print(pm10);
      Serial.print(" ug/m3");
    } else if (index > 15) {
      break;
    }
    index++;
  }
  while (mySerial.available()) mySerial.read();
  Serial.println(" }");

  Blynk.virtualWrite(V6, pm1);
  Blynk.virtualWrite(V7, pm2_5);
  Blynk.virtualWrite(V8, pm10);
}

void DHT() {
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  int f = dht.readTemperature(true);

  humidity = h;
  temperature = t;

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V5, h);

}

void Line(float pm10, float pm2_5, float pm1, float t, float h) {
  LINE.setToken(LINE_TOKEN);

  String pm10Str = String(pm10, 2);
  String pm2_5Str = String(pm2_5, 2);
  String pm1Str = String(pm1, 2);
  String tStr = String(t, 2);
  String hStr = String(h, 2);

  int time = 1;

  if (time == 10 && pm2_5 < 50){
  String message = "Notification every 2 hours \nPM10 : " + pm10Str + "\nPM2.5 : " + pm2_5Str + "\nPM0.1 : " + pm1Str + "\nTemp : " + tStr + "\nHumid : " + hStr;
    LINE.notify(message); 
    }

  do {
  Serial.println(time);
  if (time == 10 && pm2_5 < 50){
  String message = "Notification every 2 hours \nPM10 : " + pm10Str + "\nPM2.5 : " + pm2_5Str + "\nPM0.1 : " + pm1Str + "\nTemp : " + tStr + "\nHumid : " + hStr;
    LINE.notify(message); 
  }

  else if(time == 10 && pm2_5>=50){
    String Warning = "Warning : PM2.5 value exceeds 50 (ug/m3), higher than standard value.\nPM10 : " + pm10Str + "\nPM2.5 : " + pm2_5Str + "\nPM0.1 : " + pm1Str + "\nTemp : " + tStr + "\nHumid : " + hStr;
    LINE.notify(Warning);
    delay(10000);
  }  

  time++;
  delay(1000);
} while (time <= 10);

  
}

void connectWiFi() {
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.print("connected to : "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <ArduinoJson.h>

#define radar_s1 D1
#define led_radar_s1 D5
#define led_pompa_s2 D7
#define WIFI_SSID "pos bcv1"
#define WIFI_PASSWORD "bdgcityV1"
#define DATABASE_URL "https://bcv1-f450b-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyBrFK8HL0bBK7QaVm5dsQJ9Gk9Nm5-LmlU"
#define USER_EMAIL "bcv1_hwsystem@gmail.com"
#define USER_PASSWORD "rw13_bcv1_hwSystem"

WiFiClientSecure client;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String inputString = "";  
bool stringComplete = false;  
int communication = 2; 
int automation = 1; 

void setup() {
  Serial.begin(115200);
  pinMode(radar_s1, INPUT_PULLUP);
  pinMode(led_radar_s1, OUTPUT);
  pinMode(led_pompa_s2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  inputString.reserve(250);

  connectToWiFi();
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectNetwork(true);
  Firebase.begin(&config, &auth);
}

void loop() {
  if (Firebase.ready()){
    int radar_s1_state = digitalRead(radar_s1);
    if (radar_s1_state == LOW){ //radar ngambang
      digitalWrite(led_radar_s1, LOW);
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir1/Radar", 1); //0 Kosong || 1 Penuh (di firebase)
    } else { //radar ketarik
      digitalWrite(led_radar_s1, HIGH);
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir1/Radar", 0);
    }

    //MENGIRIM PESAN
    automation = Firebase.getInt(fbdo, "/ControlSystem/Automation") ? fbdo.to<int>() : 1;
    if (communication == 1) {
      StaticJsonDocument<250> doc;
      communication = 2;

      doc["radar_atas"] = radar_s1_state;
      doc["communication"] = communication;

      if (automation == 0) {
        Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay1");
        doc["relay1State"] = (fbdo.to<int>() == 1) ? 0 : 1;
        Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay2");
        doc["relay2State"] = (fbdo.to<int>() == 1) ? 0 : 1;
        Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay3");
        doc["relay3State"] = (fbdo.to<int>() == 1) ? 0 : 1;
      }

      doc["automation"] = automation;

      String data;
      serializeJson(doc, data);
      Serial.println(data);
      Serial.flush(); 
    }

    //MENERIMA PESAN
    if (stringComplete) {
      StaticJsonDocument<250> doc;
      DeserializationError error = deserializeJson(doc, inputString);

      if (!error) {
        int relay1State = doc["relay1State"];
        int relay2State = doc["relay2State"];
        int relay3State = doc["relay3State"];
        int radar1 = doc["radar1"];
        int radar2 = doc["radar2"];
        int radar3 = doc["radar3"];
        communication = doc["communication"];

        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay1", (relay1State == LOW) ? 1 : 0); //Di firebase, relay 1 = nyala || 0 = mati
        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay2", (relay2State == LOW) ? 1 : 0);
        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay3", (relay3State == LOW) ? 1 : 0);
        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorBesar1", (radar1 == LOW) ? 1 : 0); //Di firebase, 1 = penuh || 0 = kosong
        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorKecil2", (radar2 == LOW) ? 1 : 0);
        Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarPompa3", (radar3 == LOW) ? 1 : 0); //radar3 penuh = mendorong air ke atas

        if (relay3State == LOW){ //Relay 3 Nyala
          digitalWrite(led_pompa_s2, HIGH);
        } else {
          digitalWrite(led_pompa_s2, LOW);
        }
      } else {
        communication = 2;
        doc["communication"] = communication;
        String data;
        serializeJson(doc, data);
        Serial.println(data);
        Serial.flush();
      }

      inputString = "";
      stringComplete = false;
    }
  }
  serialEvent();
  delay(1000);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

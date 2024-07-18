#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ArduinoJson.h>

#define radar1 D6
#define radar2 D7
#define radar3 D0
#define relay1 D2
#define relay2 D1
#define relay3 D5

#define WIFI_SSID "pos bcv1"
#define WIFI_PASSWORD "bdgcityV1"
#define DATABASE_URL "https://bcv1-f450b-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyBrFK8HL0bBK7QaVm5dsQJ9Gk9Nm5-LmlU"
#define USER_EMAIL "bcv1_hwsystem@gmail.com"
#define USER_PASSWORD "rw13_bcv1_hwSystem"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiClientSecure client;

int currentRelay1State = HIGH;
int currentRelay2State = HIGH;
int currentRelay3State = HIGH;

int automation = 1; // 0 Manual || 1 Otomatis
int radar_atas = -1;
int currentRadar1, currentRadar2, currentRadar3;

void setup() {
  pinMode(radar1, INPUT_PULLUP);
  pinMode(radar2, INPUT_PULLUP);
  pinMode(radar3, INPUT); // Buat PULLUP EKSTERNAL
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

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
  currentRadar1 = digitalRead(radar1);
  currentRadar2 = digitalRead(radar2);
  currentRadar3 = digitalRead(radar3);

  automation = Firebase.getInt(fbdo, "/ControlSystem/Automation") ? fbdo.to<int>() : 1;

  radar_atas = Firebase.getInt(fbdo, "/ControlSystem/Reservoir1/Radar") ? fbdo.to<int>() : 1;
  if (radar_atas == 1){ //0 Kosong || 1 Penuh (di firebase)
    radar_atas == LOW;
  } else {
    radar_atas == HIGH;
  }

  if (automation == 1) {
    currentRelay1State = (currentRadar1 == LOW) ? LOW : HIGH;
    currentRelay2State = (currentRadar2 == LOW) ? LOW : HIGH;
    currentRelay3State = (radar_atas == LOW) ? LOW : HIGH; //RADAR ATAS KOSONG = mendorong air ke atas

    digitalWrite(relay1, currentRelay1State);
    digitalWrite(relay2, currentRelay2State);
    digitalWrite(relay3, currentRelay3State);
  }

  if (automation == 0) {
    currentRelay1State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay1");
    currentRelay1State = (fbdo.to<int>() == 1) ? 0 : 1;
    
    currentRelay2State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay2");
    currentRelay2State = (fbdo.to<int>() == 1) ? 0 : 1;

    currentRelay3State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay3");
    currentRelay3State = (fbdo.to<int>() == 1) ? 0 : 1;

    digitalWrite(relay1, currentRelay1State);
    digitalWrite(relay2, currentRelay2State);
    digitalWrite(relay3, currentRelay3State);
  }

  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay1", (currentRelay1State == LOW) ? 1 : 0); //Di firebase, relay 1 = nyala || 0 = mati
  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay2", (currentRelay2State == LOW) ? 1 : 0);
  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay3", (currentRelay3State == LOW) ? 1 : 0);

  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorBesar1", (currentRadar1 == LOW) ? 1 : 0); //Di firebase, 1 = penuh || 0 = kosong
  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorKecil2", (currentRadar2 == LOW) ? 1 : 0);
  Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarPompa3", (currentRadar2 == LOW) ? 1 : 0); //radar3 penuh = mendorong air ke atas

}

void sendCommunication() {
  doc["relay1State"] = currentRelay1State;
  doc["relay2State"] = currentRelay2State;
  doc["relay3State"] = currentRelay3State;
  doc["radar1"] = currentRadar1;
  doc["radar2"] = currentRadar2;
  doc["radar3"] = currentRadar2;
  doc["communication"] = communication;
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


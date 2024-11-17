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

#define WIFI_SSID "FAISAL OVIE 2"
#define WIFI_PASSWORD "1sampai0"
// #define WIFI_SSID "pos bcv1"
// #define WIFI_PASSWORD "bdgcityV1"
#define DATABASE_URL "https://bcv1-d6838-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyDGeCHzLu7OGQlCgyntd8ag5yQYxBVXdyA"
#define USER_EMAIL "bandungcityviewi@gmail.com"
#define USER_PASSWORD "rw13_bcv1_hwSystem"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiClientSecure client;

int automation = 1; 
int radar_atas = -1;
int currentRadar1, currentRadar2, currentRadar3, currentRelay1State, currentRelay2State, currentRelay3State;

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
  if (Firebase.ready()) {
    // Baca status radar dari pin digital
    currentRadar1 = digitalRead(radar1);
    currentRadar2 = digitalRead(radar2);
    // currentRadar3 = digitalRead(radar3);

    // Baca status automation dari Firebase
    automation = Firebase.getInt(fbdo, "/ControlSystem/Automation") ? fbdo.to<int>() : 1;

    radar_atas = Firebase.getInt(fbdo, "/ControlSystem/Reservoir1/Radar") ? fbdo.to<int>() : 1; // 0 = kosong di firebase

    if (automation == 1) {
      currentRelay1State = (currentRadar1 == LOW) ? LOW : HIGH;
      currentRelay2State = (currentRadar2 == LOW) ? LOW : HIGH;
      currentRelay3State = (radar_atas == 0) ? LOW : HIGH; //RADAR ATAS KOSONG = mendorong air ke atas

      digitalWrite(relay1, currentRelay1State);
      digitalWrite(relay2, currentRelay2State);
      digitalWrite(relay3, currentRelay3State);

      Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay1", (currentRelay1State == LOW) ? 1 : 0); // RELAY 1 = NYALA DI FIREBASE
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay2", (currentRelay2State == LOW) ? 1 : 0); // RELAY 0 = MATI DI FIREBASE
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/Relay3", (currentRelay3State == LOW) ? 1 : 0);
    } else if (automation == 0) {
      currentRelay1State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay1") ? fbdo.to<int>() : HIGH; // RELAY 1 = NYALA DI FIREBASE
      currentRelay2State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay2") ? fbdo.to<int>() : HIGH; // RELAY 0 = MATI DI FIREBASE
      currentRelay3State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay3") ? fbdo.to<int>() : HIGH;
      
      digitalWrite(relay1, (currentRelay1State == 1) ? LOW : HIGH);
      digitalWrite(relay2, (currentRelay2State == 1) ? LOW : HIGH);
      digitalWrite(relay3, (currentRelay3State == 1) ? LOW : HIGH);
    }

    Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorBesar1", (currentRadar1 == LOW) ? 1 : 0);
    Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarBorKecil2", (currentRadar2 == LOW) ? 1 : 0);
    Firebase.setInt(fbdo, "/ControlSystem/Reservoir2/RadarPompa3", (currentRadar2 == LOW) ? 1 : 0);
  }
  delay(100);
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

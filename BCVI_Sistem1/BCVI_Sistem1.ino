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

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiClientSecure client;
int automation = 1; 

void setup() {
  pinMode(radar_s1, INPUT_PULLUP);
  pinMode(led_radar_s1, OUTPUT);
  pinMode(led_pompa_s2, OUTPUT);
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
  if (Firebase.ready()){
    int radar_s1_state = digitalRead(radar_s1);
    if (radar_s1_state == LOW){ //radar ngambang
      digitalWrite(led_radar_s1, LOW);
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir1/Radar", 1); //0 Kosong || 1 Penuh (di firebase)
    } else { //radar ketarik
      digitalWrite(led_radar_s1, HIGH);
      Firebase.setInt(fbdo, "/ControlSystem/Reservoir1/Radar", 0);
    }

    int relay3State = Firebase.getInt(fbdo, "/ControlSystem/Reservoir2/Relay3") ? fbdo.to<int>() : 0;
    if (relay3State == 1){ //Relay 3 Nyala || Di firebase, relay 1 = nyala || 0 = mati
      digitalWrite(led_pompa_s2, HIGH);
    } else {
      digitalWrite(led_pompa_s2, LOW);
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

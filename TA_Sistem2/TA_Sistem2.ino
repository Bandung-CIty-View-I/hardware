// INI SISTEM2
#include <ArduinoJson.h>

#define radar1 D6
#define radar2 D7
#define radar3 D0
#define relay1 D2
#define relay2 D1
#define relay3 D5

int currentRelay1State = HIGH;
int currentRelay2State = HIGH;
int currentRelay3State = HIGH;

String inputString = "";  // A string to hold incoming data
bool stringComplete = false;  // Whether the string is complete
int communication = 2; // 1 untuk sistem1 ke sistem2, 2 untuk sistem2 ke sistem1
int automation = 1; // 0 Manual || 1 Otomatis
int radar_atas = -1;
int currentRadar1, currentRadar2, currentRadar3;

void setup() {
  Serial.begin(115200);
  pinMode(radar1, INPUT_PULLUP);
  pinMode(radar2, INPUT_PULLUP);
  pinMode(radar3, INPUT); // Buat PULLUP EKSTERNAL
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  currentRadar1 = digitalRead(radar1);
  currentRadar2 = digitalRead(radar2);
  currentRadar3 = digitalRead(radar3);

  if (automation == 1) {
    currentRelay1State = (currentRadar1 == LOW) ? LOW : HIGH;
    currentRelay2State = (currentRadar2 == LOW) ? LOW : HIGH;
    currentRelay3State = (radar_atas == LOW) ? LOW : HIGH; //RADAR ATAS KOSONG = mendorong air ke atas

    digitalWrite(relay1, currentRelay1State);
    digitalWrite(relay2, currentRelay2State);
    digitalWrite(relay3, currentRelay3State);
  }

  // MENGIRIM KOMUNIKASI
  if (communication == 2) {
    sendCommunication();
  }

  // MENERIMA KOMUNIKASI
  if (stringComplete) {
    StaticJsonDocument<250> doc;
    DeserializationError error = deserializeJson(doc, inputString);

    if (!error) {
      radar_atas = doc["radar_atas"];
      automation = doc["automation"];
      communication = doc["communication"];

      if (automation == 0) {
        currentRelay1State = doc["relay1State"];
        currentRelay2State = doc["relay2State"];
        currentRelay3State = doc["relay3State"];
        digitalWrite(relay1, currentRelay1State);
        digitalWrite(relay2, currentRelay2State);
        digitalWrite(relay3, currentRelay3State);
      }
    } else {
      communication = 1;
      sendCommunication();
    }

    inputString = "";
    stringComplete = false;
  }

  serialEvent();  // Call serialEvent manually
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

void sendCommunication() {
  communication = 1;

  StaticJsonDocument<250> doc;
  doc["relay1State"] = currentRelay1State;
  doc["relay2State"] = currentRelay2State;
  doc["relay3State"] = currentRelay3State;
  doc["radar1"] = currentRadar1;
  doc["radar2"] = currentRadar2;
  doc["radar3"] = currentRadar2;
  doc["communication"] = communication;

  String data;
  serializeJson(doc, data);
  Serial.println(data);
  Serial.flush();  // Ensure the data is sent
}

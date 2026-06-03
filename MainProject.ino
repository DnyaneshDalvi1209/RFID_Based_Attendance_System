#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// Pin Definitions
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D8

// RFID Module Setup
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      

// WiFi Credentials
#define WIFI_SSID "Ram"  
#define WIFI_PASSWORD "12345678"  

// Google Sheets URL
const String sheet_url = "https://script.google.com/macros/s/AKfycbyOpNj-O_MCwO01gwfK2WaqahMHCXItk0fP8O0vghMnOc-mWcWaWaZ-gD_sSmdD7_7l/exec";

// RFID Block Configuration
int blockNum = 2;
byte readBlockData[18]; // Buffer for RFID Data

// WiFi Reconnect Function
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi Reconnected!");
  }
}

// Function Prototype
void ReadDataFromBlock(int blockNum, byte readBlockData[]);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure Buzzer
  pinMode(BUZZER, OUTPUT);

  // Initialize RFID Module
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  checkWiFi(); // Ensure WiFi is always connected

  Serial.println("\nScan your card...");

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("Reading RFID Data...");
  ReadDataFromBlock(blockNum, readBlockData);

  // Print card data
  Serial.print("Card Data: ");
  Serial.println((char*)readBlockData);

  // Activate buzzer for confirmation
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  // Send Data to Google Sheets
  if (WiFi.status() == WL_CONNECTED) {
    String request_url = sheet_url + "?name=" + String((char*)readBlockData);
    request_url.trim();
    Serial.println("Request URL: " + request_url);

    HTTPClient https;
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    if (https.begin(*client, request_url)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("Server Response: %d\n", httpCode);
      } else {
        Serial.printf("HTTP GET Failed: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.println("Connection Failed!");
    }
  }

  // Halt card to allow new scans
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}

// Function to Read Data from RFID Block
void ReadDataFromBlock(int blockNum, byte readBlockData[]) { 
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication Failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  byte bufferLen = 18; // FIXED: Proper declaration
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read Failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println("Read Success!");
  }
}

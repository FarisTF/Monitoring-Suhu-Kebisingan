#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "time.h"
#include <Adafruit_I2CDevice.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// SSID dan password wifi
#define WIFI_SSID "Wifiku4"
#define WIFI_PASSWORD "21345678"

// Firebase project API Key
#define API_KEY "AIzaSyDUMajALYDs9ryeikxymW-fhPpqxlPHXrU"

// Autentikasi untuk realtime database Firebase
#define USER_EMAIL "hehe@gmail.com"
#define USER_PASSWORD "21345678"

// Untuk connect ke realtime database Firebase
#define DATABASE_URL "https://monitoring-suhu-kebising-edd81-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Pin buzzer
#define BUZZER_PIN 18

// Konstan untuk pengukuran tingkat suara
#define ADC_PIN 32                    // ADC pin connected to the MAX4466 output
#define ADC_RESOLUTION 12             // ADC resolution in bits
#define ADC_REFERENCE_VOLTAGE 3.3     // ADC reference voltage in volts

// Konstan untuk konversi voltage ke dB
#define SOUND_REF_VOLTAGE 0.775       // Reference voltage for sound level calculation (volts)
#define SOUND_SENSITIVITY 77          // Sensitivity of the MAX4466 module in mV/dB


// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;

// Path untuk tiap variabel
String kamarPath = "/kamar";
String tempPath = "/temperature";
String presPath = "/pressure";
String timePath = "/timestamp";
String isExceedTempPath = "/isExceedTemp";
String isExceedNoisePath = "/isExceedNoise";

// Batas toleransi
float batasTemp = 33;
float batasNoise = 90;

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// BMP280 sensor
Adafruit_BMP280 BMP; // I2C
float temperature;
float pressure;

// Timer (mengirim hasil pengukuran sensor setiap 15 detik)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

// Initialize BMP280
void initBMP(){
  if (!BMP.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function untuk dapet waktu
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize BMP280 sensor
  initBMP();
  initWiFi();
  configTime(0, 0, ntpServer);

  // API key (required)
  config.api_key = API_KEY;

  // User sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Realtime database URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback; //addons/TokenHelper.h

  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);

  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Mengirim hasil pengukuran oleh sensor
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Waktu saat ini
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    // Baca nilai analog dari mic
    int adcValue = analogRead(A0);
    // Konvert nilai analog ke voltage
    float voltage = (adcValue * (3.3 / 4095.0)) + 0.01;

    float soundLevel = 0;

    // Cek dulu si voltagenya
    if (voltage < 0) {
        Serial.println("Voltage too low to calculate sound level.");
    } else { 
        // Konversi ke desibel
        soundLevel = 30 * log10(voltage / SOUND_REF_VOLTAGE) + SOUND_SENSITIVITY;
        // Print ke serial monitor
        Serial.print("Sound Level (dB): ");
        Serial.println(soundLevel);
    }

    json.set(kamarPath.c_str(), "Kamar 1A");
    json.set(tempPath.c_str(), String(BMP.readTemperature()));
    Serial.print("Temperature (°C): ");
    Serial.println(BMP.readTemperature());
    json.set(presPath.c_str(), String(soundLevel));
    json.set(timePath, String(timestamp));

    json.set(isExceedTempPath.c_str(), "False"); // Default statenya tidak melebihi batas
    json.set(isExceedNoisePath.c_str(), "False");
    digitalWrite(BUZZER_PIN, LOW);

    if (BMP.readTemperature()>batasTemp){  // Pengecekan dilakukan masing masing untuk temp dan noise
        json.set(isExceedTempPath.c_str(), "True");
        digitalWrite(BUZZER_PIN, HIGH);
    }
    if (soundLevel>batasNoise){
        json.set(isExceedNoisePath.c_str(), "True");
        digitalWrite(BUZZER_PIN, HIGH);
    }

    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    Serial.println("");
  }
}
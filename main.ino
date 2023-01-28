#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define WIFI_SSID "honami"
#define WIFI_PASSWORD "147258369"
#define DATABASE_URL "https://jungyo-iot-server-default-rtdb.firebaseio.com"
#define DATABASE_SECRET "xKdHt2fM5tuAYyoiySYE6lWMUnzeo0wa7wFILauL"

int cycle = 1000;
int ckey;

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float dustDensity = 0;
float dustState = 0;
boolean DustCalculate_RUN = false;
boolean DustCalculate_Done = false;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define DATAPIN 2
DHT DHTSENSOR(DATAPIN, DHT11);
const int DUST_PIN = 14;

void setup(){
  Serial.begin(115200);
  pinMode(DUST_PIN, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connected");
  }
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
  delay(1000);
}

void loop(){
    delay(cycle);
    TemperatureCalc();
    HumidityCalc();
    DustCalc();
    checker();
    ESP.wdtDisable();
}

void TemperatureCalc(){
  float Temperature;
  Temperature = DHTSENSOR.readTemperature(false);
  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/TEMPERATURE",Temperature) == true) Serial.println("temperature stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
}

void HumidityCalc(){
  float Humidity;
  Humidity = DHTSENSOR.readHumidity(false);
  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/HUMIDIFY",Humidity) == true) Serial.println("humidity stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
}

void DustCalc(){
  duration = pulseIn(DUST_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;
  
  if ((millis() - starttime) > sampletime_ms) {
    ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    dustDensity = concentration * 100 / 13000;
    lowpulseoccupancy = 0;
  }

  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/DUST",dustDensity) == true) Serial.println("dust stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
    Serial.println(dustDensity);
}

void checker(){
  ckey = random(10000, 99999);
  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/CKEY",ckey) == true) Serial.println("ckey stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
    Serial.println(ckey);
}

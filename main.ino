#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
#define DATABASE_URL "DATABASE_URL"
#define DATABASE_SECRET "DATABASE_SECRET"

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
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("AIR-MONITORING");
  lcd.setCursor(3,1);
  lcd.print("Booting...");
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
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("TEMP");
  lcd.setCursor(6,0);
  lcd.print("HUMI");
  lcd.setCursor(11,0);
  lcd.print("DUST");
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
  lcd.setCursor(2,1);
  lcd.print(Temperature,0);
  lcd.setCursor(6,1);
  lcd.print(" ");
}

void HumidityCalc(){
  float Humidity;
  Humidity = DHTSENSOR.readHumidity(false);
  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/HUMIDITY",Humidity) == true) Serial.println("humidity stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
  lcd.setCursor(7,1);
  lcd.print(Humidity,0);
  lcd.setCursor(11,1);
  lcd.print(" ");
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
    lcd.setCursor(12,1);
    lcd.print(dustDensity,0);
    lcd.setCursor(16,1);
    lcd.print(" ");
}

void checker(){
  ckey = random(10000, 99999);
  if(Firebase.RTDB.setInt(&fbdo,"/AIR SENSOR/CKEY",ckey) == true) Serial.println("ckey stored in db");  
    else  Serial.println(fbdo.errorReason().c_str());
    Serial.println(ckey,0);
}

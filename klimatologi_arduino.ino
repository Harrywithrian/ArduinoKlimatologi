//library
#include "stdint.h"
#include "SparkFunBME280.h"
#include "BH1750FVI.h"
#include "Wire.h"

//deklarasi sensor
#define utara 2
#define tl 3
#define timur 4
#define tenggara 5
#define selatan 6
#define bd 7
#define barat 8
#define bl 9
const int trigPin = 10;
const int echoPin = 11;
BME280 sensorSuhu;
BH1750FVI sensorCahaya;
int optocoupler = 12;

//inisialisasi
int ch = 0;
String tempArah = "Kosong";
String JSON;
int validasi = 0;

//setting
void settingwindvane() {
  pinMode(utara,INPUT_PULLUP);
  pinMode(tl,INPUT_PULLUP);
  pinMode(timur,INPUT_PULLUP);
  pinMode(tenggara,INPUT_PULLUP);
  pinMode(selatan,INPUT_PULLUP);
  pinMode(bd,INPUT_PULLUP);
  pinMode(barat,INPUT_PULLUP);
  pinMode(bl,INPUT_PULLUP);
}

void settingBme280() {
  sensorSuhu.settings.commInterface = I2C_MODE;
  sensorSuhu.settings.I2CAddress = 0x76; 
  sensorSuhu.settings.runMode = 3;
  sensorSuhu.settings.tStandby = 0;
  sensorSuhu.settings.filter = 0;
  sensorSuhu.settings.tempOverSample = 1;
  sensorSuhu.settings.pressOverSample = 1;
  sensorSuhu.settings.humidOverSample = 1;
  delay(10);
  sensorSuhu.begin();
}

void settingBh1750() {
  sensorCahaya.begin();
  sensorCahaya.SetAddress(Device_Address_H);
  sensorCahaya.SetMode(Continuous_H_resolution_Mode);
}

void settingHcsr04() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

int waktuLpm(int lux) { 
  float watt = 0;
  //watt = lux / 685;
  watt = lux / 500;
  
  if (watt < 12) {
    return 0;
  } else {
    return 1;
  }
}

void setup()
{
  settingBme280();
  settingBh1750();
  settingwindvane();
  settingHcsr04();
  
  Serial.begin(9600);
}

void loop()
{
  //variable millisecond
  int jungkit = digitalRead(optocoupler);
  String arahAngin = tempArah;

  //fungsi curah hujan
  if (ch == 0) {
    if (jungkit == 1) {
      ch = 1;
    }
  }

  //fungsi arah angin
  if (digitalRead(utara)==LOW){
    arahAngin = "Utara";
  } else if (digitalRead(tl)==LOW) {
      arahAngin = "Timur Laut";
    } else if (digitalRead(timur)==LOW) {
        arahAngin = "Timur";
      } else if (digitalRead(tenggara)==LOW) {
          arahAngin = "Tenggara";
        } else if (digitalRead(selatan)==LOW) {
            arahAngin = "Selatan";
          } else if (digitalRead(bd)==LOW) {
              arahAngin = "Barat Daya";
            } else if (digitalRead(barat)==LOW) {
                arahAngin = "Barat";
              } else if (digitalRead(bl)==LOW) {
                  arahAngin = "Barat Laut";
                }
  tempArah = arahAngin;
  
  validasi = validasi+1;

  if (validasi == 100){

    //variable second
    float suhuUdara       = sensorSuhu.readTempC()- 3.5; //satuan celsius
    float tekananUdara    = sensorSuhu.readFloatPressure() / 100; //satuan mb
    float kelembabanUdara = sensorSuhu.readFloatHumidity() + 19; //dalam persen (%)
    long durasi; //jarak suara ultrasonik
    float jarak; //satuan cm
    int evaTemp; //satuan mm
    int evaporasi; //konversi mm
    int lux = sensorCahaya.GetLightIntensity(); //satuan lux
    int lpm = waktuLpm(lux); //satuan lpm jam:menit:detik
    int valueAngin = analogRead(A0) - 5;
    float mps = 0;
    int knot = 0;

    //kalibrasi kecepatan angin
    if (valueAngin < 0) {
      valueAngin = 0;
    }
    float outvoltage = valueAngin * (5.0 / 1023.0);
    mps = 6 * outvoltage;
    knot = mps * 0,514444444;

    //penentuan jarak
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    durasi = pulseIn(echoPin, HIGH);
    jarak = durasi * 0.034/2;
    evaTemp = jarak * 10;
    evaporasi = 107 - evaTemp;

    if (evaTemp > 107){
      evaporasi = 0;
    }
    
    //JSON
    JSON = "{\"data\": {";
    JSON += "\"suhu\":";
    JSON += suhuUdara;
    JSON += ", \"tekanan\":";
    JSON += tekananUdara;
    JSON += ", \"kelembaban\":";
    JSON += kelembabanUdara;
    JSON += ", \"evaporasi\":";
    JSON += evaporasi;
    JSON += ", \"lpm\":";
    JSON += lpm;
    JSON += ", \"ch\":";
    JSON += ch;
    JSON += ", \"kecepatan\":";
    JSON += mps;
    JSON += ", \"knot\":";
    JSON += knot;    
    JSON += ", \"arahangin\":\"";
    JSON += arahAngin;
    JSON += "\"}}";

    Serial.println(JSON);

    validasi = 0;
    ch = 0;
  }

  delay(10);
}

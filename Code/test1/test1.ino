// ---------------------------- LIBRARIES ----------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <ESP32Servo.h>

// ---------------------------- PIN DEFINITIONS ----------------------------
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

#define DHTPIN   15
#define DHTTYPE  DHT22

#define MQ135_PIN 34
#define ONE_WIRE_BUS 13

#define SERVO_PIN 27
#define PELTIER_PIN 25

// ---------------------------- OBJECT DECLARATIONS ----------------------------
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
BH1750 lightMeter;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;

// MQ-135 setup parameters
#define Board "ESP-32"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ135_PIN, "MQ-135");

// ---------------------------- VARIABLES ----------------------------
float dhtTemp, dhtHum;
float dsTemp;
float lightLux;
float mqValue;
bool peltierState = false;
unsigned long lastUpdate = 0;

// ---------------------------- SETUP ----------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  // TFT Display
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.println("Initializing devices...");

  // DHT22
  dht.begin();

  // I2C Sensors
  Wire.begin();
  lightMeter.begin();

  // DS18B20
  ds18b20.begin();

  // MQ135
  MQ135.init();

  // Servo
  myservo.attach(SERVO_PIN);
  myservo.write(90); // Neutral position

  // Peltier Control
  pinMode(PELTIER_PIN, OUTPUT);
  digitalWrite(PELTIER_PIN, LOW);

  tft.println("All devices ready!");
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
}

// ---------------------------- LOOP ----------------------------
void loop() {
  if (millis() - lastUpdate >= 1000) {  // Update every second
    lastUpdate = millis();

    // --- Read DHT22 ---
    dhtTemp = dht.readTemperature();
    dhtHum = dht.readHumidity();

    // --- Read DS18B20 ---
    ds18b20.requestTemperatures();
    dsTemp = ds18b20.getTempCByIndex(0);

    // --- Read BH1750 ---
    lightLux = lightMeter.readLightLevel();

    // --- Read MQ135 ---
    MQ135.update();
    mqValue = MQ135.readSensor();  // Returns ppm estimate

    // --- Control Servo (test motion) ---
    static bool servoDir = false;
    myservo.write(servoDir ? 45 : 135);
    servoDir = !servoDir;

    // --- Control Peltier ---
    if (dsTemp > 30.0) {
      digitalWrite(PELTIER_PIN, HIGH);
      peltierState = true;
    } else if (dsTemp < 25.0) {
      digitalWrite(PELTIER_PIN, LOW);
      peltierState = false;
    }

    // --- Print to Serial ---
    Serial.println("========== SENSOR DATA ==========");
    Serial.printf("DHT22 Temp: %.2f °C  Hum: %.2f %%\n", dhtTemp, dhtHum);
    Serial.printf("DS18B20 Temp: %.2f °C\n", dsTemp);
    Serial.printf("BH1750 Light: %.2f lx\n", lightLux);
    Serial.printf("MQ135 Air Quality: %.2f ppm\n", mqValue);
    Serial.printf("Peltier: %s\n", peltierState ? "ON" : "OFF");
    Serial.println("=================================");

    // --- Display on TFT ---
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 20);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_CYAN);
    tft.printf("DHT22: %.1fC  %.1f%%", dhtTemp, dhtHum);

    tft.setCursor(10, 50);
    tft.setTextColor(ILI9341_YELLOW);
    tft.printf("DS18B20: %.1fC", dsTemp);

    tft.setCursor(10, 80);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf("Light: %.0f lx", lightLux);

    tft.setCursor(10, 110);
    tft.setTextColor(ILI9341_GREEN);
    tft.printf("Air: %.0f ppm", mqValue);

    tft.setCursor(10, 140);
    tft.setTextColor(peltierState ? ILI9341_RED : ILI9341_BLUE);
    tft.printf("Peltier: %s", peltierState ? "ON" : "OFF");

    tft.setCursor(10, 180);
    tft.setTextColor(ILI9341_MAGENTA);
    tft.println("Servo moving...");
  }
}
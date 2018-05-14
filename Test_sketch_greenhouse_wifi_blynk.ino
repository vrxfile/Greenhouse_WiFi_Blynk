#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Wire.h>
#include <OneWire.h>
#include <AM2320.h>
#include <BH1750FVI.h>
#include <DallasTemperature.h>

// Точка доступа Wi-Fi
char ssid[] = "IOTIK";
char pass[] = "Terminator812";

// Датчик освещенности
BH1750FVI bh1750;

// Датчик температуры/влажности воздуха
AM2320 am2320;

// Датчик DS18B20
#define DS18B20 0
OneWire oneWire(DS18B20);
DallasTemperature ds_sensor(&oneWire);

// Выход реле
#define RELAY_PIN 2

// Выход для управления светодиодной лентой
#define LED_PIN 12

// Датчик влажности почвы
#define SOIL_SENSOR A0

// Сервомотор
#define SERVO_PWM 14
Servo servo_motor;

// Период для таймера обновления данных
#define UPDATE_TIMER 5000

// Таймер
BlynkTimer timer_update;

// Параметры IoT сервера
char auth[] = "";
IPAddress blynk_ip(139, 59, 206, 133);

void setup()
{
  // Инициализация последовательного порта
  Serial.begin(115200);
  delay(1024);

  // Инициализация Wi-Fi и поключение к серверу Blynk
  Blynk.begin(auth, ssid, pass, blynk_ip, 8442);
  Serial.println();

  // Инициализация датчика DS18B20
  ds_sensor.begin();

  // Инициализация датчика BH1750
  bh1750.begin();
  bh1750.setMode(Continuously_High_Resolution_Mode);

  // Инициализация датчика AM2320
  am2320.begin();

  // Инициализация выхода реле
  pinMode(RELAY_PIN, OUTPUT);

  // Инициализация выхода светодиодной ленты
  pinMode(LED_PIN, OUTPUT);

  // Инициализация порта для управления сервомотором
  servo_motor.attach(SERVO_PWM);
  servo_motor.write(90);
  delay(1024);

  // Инициализация таймера
  timer_update.setInterval(UPDATE_TIMER, readSendData);
}

void loop()
{
  Blynk.run();
  timer_update.run();
}

// Считывание датчиков и отправка данных на сервер Blynk
void readSendData()
{
  if (am2320.measure())
  {
    float air_temp = am2320.getTemperature();            // Считывание температуры воздуха
    float air_hum = am2320.getHumidity();                // Считывание влажности воздуха
    Serial.print("Air temperature = ");
    Serial.println(air_temp);
    Serial.print("Air humidity = ");
    Serial.println(air_hum);
    Blynk.virtualWrite(V0, air_temp); delay(25);        // Отправка данных на сервер Blynk
    Blynk.virtualWrite(V1, air_hum); delay(25);         // Отправка данных на сервер Blynk
  }
  else
  {
    float air_temp = -127.0;
    float air_hum = -127.0;
    Serial.println("Error reading AM2320 sensor!");
    Blynk.virtualWrite(V0, air_temp); delay(25);        // Отправка данных на сервер Blynk
    Blynk.virtualWrite(V1, air_hum); delay(25);         // Отправка данных на сервер Blynk
  }

  float light = bh1750.getAmbientLight();               // Считывание датчика света
  Serial.print("Light = ");
  Serial.println(light);
  Blynk.virtualWrite(V2, light); delay(25);             // Отправка данных на сервер Blynk

  ds_sensor.requestTemperatures();                      // Считывание температуры почвы
  float soil_temp = ds_sensor.getTempCByIndex(0);
  Serial.print("Soil temperature = ");
  Serial.println(soil_temp);
  Blynk.virtualWrite(V3, soil_temp); delay(25);         // Отправка данных на сервер Blynk

  float soil_hum = analogRead(SOIL_SENSOR) / 1023.0 * 100.0;  // Считывание влажности почвы
  Serial.print("Soil moisture = ");
  Serial.println(soil_hum);
  Blynk.virtualWrite(V4, soil_hum); delay(25);          // Отправка данных на сервер Blynk
}

// Управление реле с Blynk
BLYNK_WRITE(V5)
{
  // Получение управляющего сигнала с сервера
  int relay_ctl = param.asInt();
  Serial.print("Relay power: ");
  Serial.println(relay_ctl);

  // Управление реле (помпой)
  digitalWrite(RELAY_PIN, relay_ctl);
}

// Управление освещением с Blynk
BLYNK_WRITE(V6)
{
  // Получение управляющего сигнала с сервера
  int light_ctl = param.asInt();
  Serial.print("Light power: ");
  Serial.println(light_ctl);

  // Управление светодиодной лентой
  analogWrite(LED_PIN, light_ctl * 10.23);
}

// Управление сервомотором с Blynk
BLYNK_WRITE(V7)
{
  // Получение управляющего сигнала с сервера
  int servo_ctl = constrain((param.asInt() + 90), 90, 135);
  Serial.print("Servo angle: ");
  Serial.println(servo_ctl);

  // Управление сервомотором
  servo_motor.write(servo_ctl);
}


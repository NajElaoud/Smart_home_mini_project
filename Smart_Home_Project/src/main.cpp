//* Template ID (only if using Blynk.Cloud)
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL2Fqpq1dXH"
#define BLYNK_TEMPLATE_NAME "smart home"
#define BLYNK_AUTH_TOKEN "eWVp4N7nibFjb5cihfrVReV-odIXRbQT"

//* Your WiFi credentials.
char ssid[] = "Galaxy A125533";
char pass[] = "11111111";

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>
// #include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Preferences pref;

// SoftwareSerial SIM800L(6, 7); // RX | TX

//* Define the GPIO connected with Relays and Inputs
#define RelayPin1 26 // D26
#define RelayPin2 25 // D25
#define RelayPin3 32 // D32
#define RelayPin4 33 // D33

#define FLAME_S_D0 14 // D14
#define GAZ_S_D0 27   // D27
#define FLAME_S_A0 35 // D35
#define GAZ_S_A0 34   // D34
#define EXT_INPUT1 13 // D13
#define EXT_INPUT2 12 // D12

#define wifi_Led 2 // D2
#define GSM_Led 5  // D5
#define Buzzer 18  // D18

//* Virtual pins select
#define VPIN_BUTTON_1 V1 // vP1_ For relay n°1
#define VPIN_BUTTON_2 V2 // vP2_ For relay n°2
#define VPIN_BUTTON_3 V3 // vP3_ For relay n°3
#define VPIN_BUTTON_4 V4 // vP4_ For relay n°4
#define VPIN_BUTTON_C V5 // vP5_ For all relays OFF
#define VPIN_FLAME V6    // vP6_ For flame sensor input
#define VPIN_GAZ V7      // vP7_ For gas sensor inputq

//* Relay initial State
bool toggleState_1 = LOW; // Toggle state for relay 1
bool toggleState_2 = LOW; // Toggle state for relay 2
bool toggleState_3 = LOW; // Toggle state for relay 3
bool toggleState_4 = LOW; // Toggle state for relay 4

bool flame_state = false;
float gaz_value = 0.;
float gaz_value_disp = 0.;
int wifiFlag = 0;

//* State change
#define TURN_ON 1
#define TURN_OFF 0

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;

void all_SwitchOff();

//* When App button is pushed - switch the state
BLYNK_WRITE(VPIN_BUTTON_1)
{
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, !toggleState_1);
  pref.putBool("Relay1", toggleState_1);
}

BLYNK_WRITE(VPIN_BUTTON_2)
{
  toggleState_2 = param.asInt();
  digitalWrite(RelayPin2, !toggleState_2);
  pref.putBool("Relay2", toggleState_2);
}

BLYNK_WRITE(VPIN_BUTTON_3)
{
  toggleState_3 = param.asInt();
  digitalWrite(RelayPin3, !toggleState_3);
  pref.putBool("Relay3", toggleState_3);
}

BLYNK_WRITE(VPIN_BUTTON_4)
{
  toggleState_4 = param.asInt();
  digitalWrite(RelayPin4, !toggleState_4);
  pref.putBool("Relay4", toggleState_4);
}

BLYNK_WRITE(VPIN_BUTTON_C)
{
  all_SwitchOff();
}

void checkBlynkStatus()
{ // called every 3 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false)
  {
    wifiFlag = 1;
    digitalWrite(wifi_Led, LOW);
    Serial.println("Blynk Not Connected");
  }
  if (isconnected == true)
  {
    wifiFlag = 0;
    digitalWrite(wifi_Led, HIGH);
    Serial.println("Blynk Connected");
  }
}

BLYNK_CONNECTED()
{
  // update the latest state to the server
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
  Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);

  Blynk.syncVirtual(VPIN_FLAME);
  Blynk.syncVirtual(VPIN_GAZ);
}

void readSensor()
{
  int flame = digitalRead(FLAME_S_D0);
  if (isnan(flame))
  {
    Serial.println("Failed to read data from FLAME sensor!");
    return;
  }
  else
  {
    flame_state = flame;
    if (flame == 1)
    {
      Serial.printf("!! FLAME Detected!!\n");
    }
  }

  int gaz = analogRead(GAZ_S_A0);
  gaz_value_disp = map(gaz, 0, 1023, 0, 100);
  if (isnan(gaz))
  {
    Serial.println("Failed to read data from MQ2 sensor!");
    return;
  }
  else
  {
    gaz_value = gaz;
    Serial.printf("MQ2 sensor value: %.2f\n", gaz_value);
  }
}

void sendSensorData()
{
  readSensor();
  //* GAS_SENSOR
  Blynk.virtualWrite(VPIN_FLAME, flame_state);
  if (gaz_value > 700)
  {
    Blynk.logEvent("Warning! Gas leak detected");
    digitalWrite(RelayPin2, TURN_ON);
    digitalWrite(Buzzer, HIGH);
  }
  Blynk.virtualWrite(VPIN_GAZ, gaz_value_disp);

  //* FLAME_SENSOR
  if (flame_state == HIGH)
  {
    digitalWrite(Buzzer, LOW);
  }
  else if (flame_state == LOW)
  {
    Blynk.logEvent("Warning! Fire was detected");
    digitalWrite(Buzzer, HIGH);
  }
  Blynk.virtualWrite(VPIN_FLAME, flame_state);
}

void all_SwitchOff()
{
  //? Relay_1
  toggleState_1 = 0;
  digitalWrite(RelayPin1, HIGH);
  pref.putBool("Relay1", toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(100);
  //? Relay_2
  toggleState_2 = 0;
  digitalWrite(RelayPin2, HIGH);
  pref.putBool("Relay2", toggleState_2);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  delay(100);
  //? Relay_3
  toggleState_3 = 0;
  digitalWrite(RelayPin3, HIGH);
  pref.putBool("Relay3", toggleState_3);
  Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
  delay(100);
  //? Relay_4
  toggleState_4 = 0;
  digitalWrite(RelayPin4, HIGH);
  pref.putBool("Relay4", toggleState_4);
  Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
  delay(100);
  //? Sensor input
  Blynk.virtualWrite(VPIN_FLAME, flame_state);
  Blynk.virtualWrite(VPIN_GAZ, gaz_value);
}

void getRelayState()
{
  Serial.println("reading data from NVS");

  toggleState_1 = pref.getBool("Relay1", TURN_OFF);
  digitalWrite(RelayPin1, !toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(200);

  toggleState_2 = pref.getBool("Relay2", TURN_OFF);
  digitalWrite(RelayPin2, !toggleState_2);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  delay(200);

  toggleState_3 = pref.getBool("Relay3", TURN_OFF);
  digitalWrite(RelayPin3, !toggleState_3);
  Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
  delay(200);

  toggleState_4 = pref.getBool("Relay4", TURN_OFF);
  digitalWrite(RelayPin4, !toggleState_4);
  Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
  delay(200);
}

void setup()
{
  Serial.begin(9600);
  // SIM800L.begin(9600);
  //  Open namespace in read-write mode
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);

  pinMode(wifi_Led, OUTPUT);

  // During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !toggleState_1);
  digitalWrite(RelayPin2, !toggleState_2);
  digitalWrite(RelayPin3, !toggleState_3);
  digitalWrite(RelayPin4, !toggleState_4);

  digitalWrite(wifi_Led, LOW);

  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensorData);   // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);

  getRelayState(); // fetch data from NVS Flash Memory
  //  delay(1000);
}

void loop()
{
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}
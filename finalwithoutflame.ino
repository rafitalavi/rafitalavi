#include <SoftwareSerial.h>
SoftwareSerial SIM900A(10, 11);

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint32_t delayMS;
float lastTemp = 0;
unsigned long lastTempTime = 0;
bool overheatDetected = false;
int relayPin = 9; // Relay connected to pin 9
int smsc = 0;
int relayPin2 = 7;

void setup() {
  Serial.begin(9600);
  SIM900A.begin(9600);
  lcd.begin();
  lcd.backlight();
  dht.begin();
  pinMode(relayPin, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin, HIGH); // Initialize relay to OFF state
  digitalWrite(relayPin2, HIGH); // Initialize relay2 to OFF state
  
  Serial.println(F("Welcome to the System"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELLCOME THE ");
  
  lcd.setCursor(0, 1);
  lcd.print("SMART PROTECTION");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(" Cheking  All   ");

  

  lcd.setCursor(0, 1);
  lcd.print("  Connection    ");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(" Establishing   ");

  lcd.setCursor(0, 1);
  lcd.print(" GPRS NETWORK   ");
  delay(1000);

  




  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  //delayMS = sensor.min_delay / 1000;
  // Start GSM module
  SIM900A.begin(9600);
  // Initialize the GSM module
}

int readGas() {
  int gasValue = analogRead(A0);
  Serial.print("Gas Value: ");
  Serial.println(gasValue);
  return gasValue;
}

float dht22() {
  delay(delayMS);
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float currentTemp = event.temperature;

  // Display on Serial Monitor
  Serial.print("Temperature: ");
  if (isnan(currentTemp)) {
    Serial.println("Error");
  } else {
    Serial.print(currentTemp);
    Serial.println("C");
  }

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp:     ");
  if (isnan(currentTemp)) {
    lcd.print("Error");
  } else {
    lcd.print(currentTemp);
    lcd.print("C");
  }

  return currentTemp;
}

float dht221() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);

  Serial.print("Humidity:  ");
  if (isnan(event.relative_humidity)) {
    Serial.println("Error");
  } else {
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }

  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  if (isnan(event.relative_humidity)) {
    lcd.print("Error");
  } else {
    lcd.print(event.relative_humidity);
    lcd.print("%");
  }

  return event.relative_humidity;
}
void statusUpdate(int tem, int gas, float humidity)
{
  String message;
  message = "Temperature: " + String(tem) + " Gas: " + String(gas) + " Humidity: " + String(humidity) + " Machine is running fine, No fire, No Gas Leakage found";
sendSMS(message);

}

void tempWarning(int temf) {
  if (temf == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("OVERHEATING!");
    Serial.println("OVERHEATING!");
     // Send SMS for overheating
    digitalWrite(relayPin2, LOW);

    digitalWrite(relayPin, LOW);
    sendSMS("Overheating detected!"); // Turn OFF the relay
  } else {
    digitalWrite(relayPin, HIGH);
    digitalWrite(relayPin2, HIGH); // Turn ON relay2
  }
}
void gasWarning(int gasF)
{
  if (gasF == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("GAS LEAKAGE");
     // Send SMS for gas leakage
    digitalWrite(relayPin2, LOW);

    digitalWrite(relayPin, LOW); // Turn OFF the relay
    sendSMS("Gas leakage detected!");
    

  } else {
    digitalWrite(relayPin2, HIGH);
    digitalWrite(relayPin, HIGH); // Turn OFF the relay
  }
  gasF =0;
}

void sendSMS(String message) {
  Serial.println("Sending SMS...");
 
  SIM900A.println("AT+CMGF=1"); // Sets the GSM Module in Text Mode
  delay(1000);
  SIM900A.println("AT+CMGS=\"+8801631381077\"\r"); // Mobile phone number to send message
  delay(1000);

  SIM900A.print(message);
  delay(100);
  SIM900A.write(26); // ASCII code of Ctrl+Z
 
  delay(5000); // Wait for the SMS to be sent

  // Read the serial response and check if it contains "OK"
  String response = SIM900A.readString();
  if (response.indexOf("OK") != -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("SMS sent successfully.");
    lcd.setCursor(0, 1);
    lcd.println(" successfully.");
    Serial.println("SMS sent successfully.");
  } else {
    Serial.println("SMS sending failed.");
  }
}


void loop() {
  // Temperature and Humidity Section
//  delay(delayMS);
  int gas = readGas();
  float temp = dht22();
  float humidity = dht221();
  int tempf = 0;
  int gasF = 0;
 
  // Update gasF and tempf based on conditions
  if (temp > 36) {
    tempf = 1;
    tempWarning(tempf);
   
  }
  else {
    tempWarning(0); // No gas leakage
  }
 
  // You need to define the condition for gasF based on your specific requirements
  // For example:
  if (gas > 450) {
    gasF = 1;
    gasWarning(gasF);
  }
  else {
    gasWarning(0); // No gas leakage
  }

  if (smsc == 50) {
    statusUpdate(temp, gas, humidity);
    smsc = 0;
  }
  smsc = smsc+1;
  Serial.print("Time: ");
  Serial.print(smsc);
  Serial.println("sec");
  delay(1000);
}
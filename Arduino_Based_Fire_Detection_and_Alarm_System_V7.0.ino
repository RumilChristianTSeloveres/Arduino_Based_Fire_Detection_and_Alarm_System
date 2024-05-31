// ############### INPUT DEVICES ################ //

//Declaring Variables for Fire I/O
const int Blue_Low_Danger = 4;
const int Green_Moderate_Danger = 5;
const int Yellow_High_Danger = 6;
const int Red_Extreme_Danger = 7;
int Fire_Sensor = A0;
int ScaledflameSensorReading = 0;
int Fire_Status = 0;
int Temperature_Status = 0;
int Smoke_Status = 0;
String FireMsg, SmokeMsg,HeatIndexMsg;
// lowest and highest sensor readings:
const int sensorMin = 0;     //  sensor minimum
const int sensorMax = 1024;  // sensor maximum

//Declaring Variables for Temperature I/O
//#include "DHT.h"
//#define DHTPIN 2       // Digital pin connected to the DHT sensor
//#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
float h = 0;           //Humidity
float t = 0;           //Temperature as Celcius
float f = 0;           //Temperature as Fahrenheit
float hif = 0;         //Heat index in Fahrenheit
float hic = 0;         //Heat index in Celcius
// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

//Declaring Variables for Smoke sensor I/O
#define sensorPin A1
unsigned int ScaledSmokeValue = 0;

// ############### OUTPUTS DEVICES ################ //
const int Buzzer = 13;  // connected to pin 13

// Sim800L GSM Module //
#include <SoftwareSerial.h>
SoftwareSerial mySim(9, 8); // SIM800L Rx(kadto naay resistor unya connected sa D2) & Tx (diretso ra sa D3 wla syay resistor)

void setup() {
  // put your setup code here, to run once:
  pinMode(Blue_Low_Danger, OUTPUT);
  pinMode(Green_Moderate_Danger, OUTPUT);
  pinMode(Yellow_High_Danger, OUTPUT);
  pinMode(Red_Extreme_Danger, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Fire_Sensor, INPUT);
  Serial.begin(9600);
  
  Serial.println(F("DHT22 connection test!"));
  Serial.println("Warming up the Smoke Sensor! . . .");
  delay(20000);  //delay by 20 seconds to warm up the MQ-2 Smoke Sensor
  dht.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  readFireSensor();
  Temperature();
  SmokeSensor();
  HeatIndex();
  Serial.println();
  Probabilities();
}
//  This function returns the analog soil moisture measurement
void readFireSensor() {
  //delay(10);              // Allow power to settle
  int sensorReading = analogRead(A0);

  // map the sensor range (four options):
  // ex: 'long  int map(long int, long int, long int, long int, long int)'
  int flameSensorReading = map(sensorReading, sensorMin, sensorMax, 1024, 0);
  ScaledflameSensorReading = flameSensorReading;
  Serial.print("Flame Sensor Value: ");
  Serial.print(ScaledflameSensorReading);
  Serial.print(" units");

  if (ScaledflameSensorReading >= 0 && ScaledflameSensorReading < 400) {
    Serial.println(" -> ** No  Fire Detected **");
    Fire_Status = 1;
    FireMsg = "No  Fire Detected";
  } else if (ScaledflameSensorReading >= 400 && ScaledflameSensorReading < 500) {
    Serial.println(" -> ** Possibility of Fire Detected **");
    Fire_Status = 2;
    FireMsg = "Possibility of Fire Detected";
  } else if (ScaledflameSensorReading >= 500 && ScaledflameSensorReading < 600) {
    Serial.println(" -> ** Fire Detected **");
    Fire_Status = 3;
    FireMsg = "Fire Detected";
  } else {
    Serial.println(" -> ** Fire Getting Closer **");
    Fire_Status = 4;
    FireMsg = "Fire Getting Closer";
  }
}

void Temperature() {
  // Wait a few seconds between measurements.
  delay(1000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  if (t < 40) {
    Temperature_Status = 1;
  } else if (t >= 40 && t <= 50) {
    Temperature_Status = 2;
  } else if (t > 50 && t <= 70) {
    Temperature_Status = 3;
  } else {
    Temperature_Status = 4;
  }
}

void SmokeSensor() {
  unsigned int sensorValue = analogRead(sensorPin);
  unsigned int outputValue = map(sensorValue, 0, 1023, 0, 1000000);  //convert to ppm
  ScaledSmokeValue = outputValue / 1000;                             //scale value down
  Serial.print("Analog output: ");
  Serial.print(ScaledSmokeValue);
  Serial.print(" ppm");
  if (ScaledSmokeValue <= 50) {
    Serial.println(" -> Smoke at safe level!");
    Smoke_Status = 1;
    //SmokeMsg = "Smoke at safe level";
  } else if (ScaledSmokeValue > 50 && ScaledSmokeValue <= 55) {
    Serial.println(" -> Smoke Detected!!!");
    Smoke_Status = 2;
    SmokeMsg = "Smoke Detected";
  } else if (ScaledSmokeValue > 55 && ScaledSmokeValue <= 60) {
    Serial.println(" -> Increase of Smoke Detected!!!");
    Smoke_Status = 3;
    SmokeMsg ="Increase of Smoke Detected";
  } else {
    Serial.println(" -> Large Concentration of Smoke Detected!");
    Smoke_Status = 4;
    SmokeMsg ="Large Concentration of Smoke";
  }
  delay(500);
}


void Probabilities() {

  //###### 111 to 144 ######//
  if (Fire_Status == 1 && Temperature_Status == 1 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, HIGH);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
  } else if (Fire_Status == 1 && Temperature_Status == 1 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    do{}while(Fire Status!=2 Temperature_Status!=2);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 1 && Temperature_Status == 1 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 1 && Temperature_Status == 1 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 2 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 1 && Temperature_Status == 2 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 1 && Temperature_Status == 2 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 1 && Temperature_Status == 2 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 3 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 1 && Temperature_Status == 3 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 1 && Temperature_Status == 3 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 1 && Temperature_Status == 3 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 4 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 4 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 4 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 1 && Temperature_Status == 4 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  }
  
  //###### 211 to 244 ######//
  else if (Fire_Status == 2 && Temperature_Status == 1 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 2 && Temperature_Status == 1 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 2 && Temperature_Status == 1 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 2 && Temperature_Status == 1 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 2 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 2 && Temperature_Status == 2 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, HIGH);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,1000,200);
  } else if (Fire_Status == 2 && Temperature_Status == 2 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 2 && Temperature_Status == 2 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 3 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 2 && Temperature_Status == 3 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 2 && Temperature_Status == 3 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 2 && Temperature_Status == 3 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 4 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 4 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 4 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 2 && Temperature_Status == 4 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  }

  //###### 311 to 344 ######//
  else if (Fire_Status == 3 && Temperature_Status == 1 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 1 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 1 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 1 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 2 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 2 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 2 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 2 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 3 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 3 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 3 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, HIGH);
    digitalWrite(Red_Extreme_Danger, LOW);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,2000,1000); //(pin, Frequency, duration)
  } else if (Fire_Status == 3 && Temperature_Status == 3 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 4 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 4 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 4 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 3 && Temperature_Status == 4 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  }

    //###### 411 to 444 ######//
  else if (Fire_Status == 4 && Temperature_Status == 1 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 1 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 1 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 1 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 2 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 2 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 2 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 2 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 3 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 3 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 3 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 3 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 4 && Smoke_Status == 1) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 4 && Smoke_Status == 2) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 4 && Smoke_Status == 3) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  } else if (Fire_Status == 4 && Temperature_Status == 4 && Smoke_Status == 4) {
    //Serial.println(" -> ** No  Fire Detected **");
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, HIGH);
    Serial.println(Fire_Status);
    Serial.println(Temperature_Status);
    Serial.println(Smoke_Status);
    tone(Buzzer,5000,2000);
  }
  else {
    //Do nothing
    digitalWrite(Blue_Low_Danger, LOW);
    digitalWrite(Green_Moderate_Danger, LOW);
    digitalWrite(Yellow_High_Danger, LOW);
    digitalWrite(Red_Extreme_Danger, LOW);
  }
}

void GSMmodule(){
  mySerial.begin(9600); // baud rate default sa sim800L
  delay(1000); // give time to log on to network.
  mySerial.println("AT+CMGF=1");    // Set the module to SMS mode 
  delay(100);
  mySerial.println("AT+CMGS=\"+639661552780\"");    // Replace with your phone number
  delay(100);
  mySerial.println("Fire Detection & Alarm System by Ahyeeza");
  mySerial.println("Smoke: " + String(ScaledSmokeValue) + " ppm" + "\nHumidity: " + String(h) + "%");
  mySerial.println("Temp: " + String(t) + " 'C" + "\nHeat Index: " + String(hic)+" 'C");
  mySerial.println("Fire Detection: " + String(ScaledflameSensorReading) + " units" + "\nInterpretations: " );
  mySerial.print(FireMsg +",\n" SmokeMsg +",\n"HeatIndexMsg+".");
 //a.concat(b);
//  mySerial.println("Large Concentration of Smoke, ");
//  mySerial.println("Extreme Caution.");
  delay(100);
  mySerial.println((char)26);    // End AT command with a ^Z, ASCII code 26
  delay(100);
  mySerial.println();
  delay(5000); // give module time to send SMS
  mySerial.flush(); // Clear anything left in the serial buffer
}

void HeatIndex(){
  if (hic < 80) {
    HeatIndexMsg = "Safe";
  } else if (hic >= 80 && hic <= 90) {
    HeatIndexMsg = "Caution";
  } else if (hic >= 91 && t <= 103) {
    HeatIndexMsg = "Extreme Caution";
  } else if (hic >= 104 && t <= 124) {
    HeatIndexMsg = "Danger";
  }else {
   HeatIndexMsg = "Extreme Danger";
  }
}

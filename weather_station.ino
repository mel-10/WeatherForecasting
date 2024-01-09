#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


 
#include "index.h" //Our HTML webpage contents with javascripts
#include "DHTesp.h" //DHT11 Library for ESP
 
#define LED 2 //On board LED
#define DHTpin 14 //D5 of NodeMCU is GPIO14

#define BMP_SDA 4   // Define the SDA pin for BMP280 on GPIO4
#define BMP_SCL 5   // Define the SCL pin for BMP280 on GPIO5

 
 
#define ALTITUDE 310.5 // Altitude in meters

#define USER_EMAIL "mkaramkar@gmail.com"
#define USER_PASSWORD "saymyname"



Adafruit_BMP280 pressure;


 
DHTesp dht;

#define DATABASE_URL "weatherforecasting-20449-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyB3-bQdi-ON74ZnRBq5dbGD7rolqu6hDdc"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;



 
//SSID and Password of your WiFi router
const char* ssid = "whatever";
const char* password = "malika123";
 
ESP8266WebServer server(80); //Server on port 80
 
void handleRoot() {
String s = MAIN_page; //Read HTML contents
server.send(200, "text/html", s); //Send web page
}
 
float humidity, temperature;
 
void handleADC() {
  char status;
  double T, P, p0, a;
  double Tdeg, Tfar, phg, pmb;

  
  // Read temperature
  status = pressure.readTemperature();
  if (status != 0) {
   
 // Wait for the measurement to complete
    delay(status);
    T = pressure.readTemperature();

    


    // Print temperature in degrees Celsius and Fahrenheit
    Serial.print("Temperature: ");
    Serial.print(T, 2);
    Tdeg = T;
    Serial.print(" °C, ");
    Tfar = (9.0 / 5.0) * T + 32.0;
    Serial.print(Tfar, 2);
    Serial.println(" °F");


    // Read pressure
    P = pressure.readPressure(); // Corrected function call



    // Print absolute pressure in millibars and inches of mercury
    Serial.print("Absolute Pressure: ");
    Serial.print(P / 100.0, 2);
    pmb = P / 100.0;
    phg = P * 0.0295333727;
    Serial.print(" mb, ");
    Serial.print(phg, 2);
    Serial.println(" inHg");


    // Calculate relative (sea-level) pressure using the barometric formula
    p0 = P / pow(1 - (ALTITUDE / 44330.0), 5.255); // You can adjust the ALTITUDE value as needed
    Serial.print("Relative (Sea-level) Pressure: ");
    Serial.print(p0 / 100.0, 2); // in millibars
    Serial.print(" mb, ");
    Serial.print(p0 * 0.0295333727, 2); // in inches of mercury
    Serial.println(" inHg");

  
    
  } else {
    Serial.println("Error retrieving temperature measurement");
  }
  
   int rain = analogRead(A0);
   int BoolRain;
  if(rain>200){
    BoolRain=1;
  }else{
    BoolRain=0;
  }
  
   
   humidity = dht.getHumidity();
  temperature = dht.getTemperature();

   
  


  // Create JSON data
  String data = "{\"Rain\":\"" + String(rain) +
                "\",\"Pressuremb\":\"" + String(pmb) +
                "\",\"Pressurehg\":\"" + String(phg) +
                "\", \"Temperature\":\"" + String(temperature) + // Using Tdeg for temperature in degrees Celsius
                "\", \"Humidity\":\"" + String(humidity) + "\"}";
 
 


  digitalWrite(LED, !digitalRead(LED)); // Toggle LED on data request ajax
  server.send(200, "text/plain", data); // Send ADC value, temperature, and humidity JSON to client ajax request

   delay(dht.getMinimumSamplingPeriod());



  Serial.print("H:");
  Serial.println(humidity);   
  Serial.print("T:");
  Serial.println(temperature);     
  Serial.print("R:");
  Serial.println(rain);

  //Firebase connection

  if (Firebase.ready()){

    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "/Sensor/Temperature", temperature)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/Sensor/Humidity", humidity)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/Sensor/Pressure", pmb)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
     // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "/Sensor/Rain", BoolRain)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
  }


  

 
  
  delay(5000);





  
}


void setup()
{
Serial.begin(115200);
delay(1000);
 
// dht11 Sensor
 
dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 14
pinMode(LED,OUTPUT);
 
//BMP280 Sensor
 if (!pressure.begin(0X76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while(1);
  } else{
    Serial.println("BMP280 init success");
  }

  

 
WiFi.begin(ssid, password); //Connect to your WiFi router

 
// Wait for connection
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
 
//If connection successful show IP address in serial monitor
Serial.println("");
Serial.print("Connected to ");
Serial.println(ssid);
Serial.print("IP address: ");
Serial.println(WiFi.localIP()); //IP address assigned to your ESP


    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

/* Assign the api key (required) */
  config.api_key = API_KEY ;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL ;
  
  /* Sign up */
  

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


 
server.on("/",HTTP_GET, handleRoot); //Which routine to handle at root location. This is display page
server.on("/readADC",HTTP_GET, handleADC); //This page is called by java Script AJAX

 
server.begin(); //Start server
Serial.println("HTTP server started");
}
 
void loop()
{
server.handleClient(); //Handle client requests
}


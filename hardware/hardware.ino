//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <rom/rtc.h> 
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE
#include <FastLED.h>
#include "DHT.h"

#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

 
// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1 

#define NUM_LEDS 7
#define LED_PIN 26

// DEFINE THE CONTROL PINS FOR THE DHT22 
#define DHTPIN 25
#define DHTTYPE DHT22

// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620154033";                    // Add your ID number here
static const char* subtopic[]    = {"620154033_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
//static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String 
static const char* mqtt_server   = "dbs.msjrealtms.com";         // Broker IP address or Domain name as a String 

static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "MonaConnect";     // Add your Wi-Fi ssid
const char* password   = "";    // Add your Wi-Fi password 


// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
 

/* Declare your functions below */ 
double convert_Celsius_to_fahrenheit(double c);
double convert_fahrenheit_to_Celsius(double f);
double calcHeatIndex(double Temp, double Humid);

/* Init class Instances for the DHT22 etcc */
DHT dht(DHTPIN, DHTTYPE);  

CRGB leds[NUM_LEDS];

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables 


void setup() {
  Serial.begin(115200);  // INIT SERIAL   

  // INITIALIZE ALL SENSORS AND DEVICES
  
  /* Add all other necessary sensor Initializations and Configurations here */
  dht.begin();
  //pinMode(DHTPIN, INPUT);
  //pinMode(LED_PIN, OUTPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  initialize();     // INIT WIFI, MQTT & NTP 
  // vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS INT THIS LAB, THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
 }
  


void loop() {
    // put your main code here, to run repeatedly:       
    vTaskDelay(1000 / portTICK_PERIOD_MS);    
}




  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed 
        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
           
    for( ;; ) {
          // #######################################################
          // ## This function must PUBLISH to topic every second. ##
          // #######################################################
   
          // 1. Read Humidity and save in variable below
          double h = dht.readHumidity();
           
          // 2. Read temperature as Celsius   and save in variable below 
          double t = dht.readTemperature();

          if(isNumber(t)){
              // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’

              // 1. Create JSon object
              StaticJsonDocument<1000> doc;
              // 2. Create message buffer/array to store serialized JSON object
              char message[1100]  = { 0 };
              // 3. Add key:value pairs to JSon object based on above schema
              doc["id"]                 = "620154033";
              doc["timestamp"]          = getTimeStamp();
              doc["temperature"]        = t;
              doc["humidity"]           = h;
              doc["heatindex"]          = calcHeatIndex(t,h);
              // 4. Seralize / Covert JSon object to JSon string and store in message array
              serializeJson(doc, message); 
              // 5. Publish message to a topic subscribed to by both backend and frontend                
              if(mqtt.connected()){
                publish(pubtopic, message);
              }
          }  
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

 

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  // PROCESS MESSAGE
  const char* type = doc["type"]; 
  // ‘{"type": "controls", "brightness": 255, "leds": 7, "color": { "r": 255, "g": 255, "b": 255, "a": 1 } }’
 

  if (strcmp(type, "controls") == 0){
    // 1. EXTRACT ALL PARAMETERS: NODES, RED,GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT
    int brightness  = doc["brightness"]; 
    int nodes       = doc["leds"];
    int red         = doc["color"]["r"];
    int green       = doc["color"]["g"];
    int blue        = doc["color"]["b"];
    // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF NODES. Ex IF NODES = 2, TURN ON 2 LED(s)
    for (int i = 0; i < nodes; i++)
    {
      leds[i] = CRGB(red, green, blue);
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(50);
    }
    // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
    for(int j = nodes; j < NUM_LEDS; j++)
    {
      leds[j] = CRGB::Black;
      FastLED.setBrightness( brightness );
      FastLED.show();
      delay(50);
    }
  }
}

bool publish(const char *topic, const char *payload){   
    bool res = false;
    try{
      res = mqtt.publish(topic,payload);
      // Serial.printf("\nres : %d\n",res);
      if(!res){
        res = false;
        throw false;
      }
    }
    catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
    }
  return res;
}



//***** Complete the util functions below ******

double convert_Celsius_to_fahrenheit(double c){    
  // CONVERTS INPUT FROM °C TO °F. RETURN RESULTS
  return (c * 9.0/5.0) + 32;
}

double convert_fahrenheit_to_Celsius(double f){    
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT 
  return (5.0/9.0) * (f - 32);   
}

double calcHeatIndex(double Temp, double Humid){
    // CALCULATE AND RETURN HEAT INDEX USING EQUATION FOUND AT https://byjus.com/heat-index-formula/#:~:text=The%20heat%20index%20formula%20is,an%20implied%20humidity%20of%2020%25

  double ft = convert_Celsius_to_fahrenheit(Temp);
  double hI = -42.379 + (2.04901523 * ft) + (10.14333127 * Humid) + (-0.22475541 * ft * Humid) + (-0.00683783 * pow(ft, 2))  + (-0.05481717 * pow(Humid,2)) + (0.00122874 * pow(ft, 2) * Humid)  + (0.00085282 * ft * pow(Humid,2)) + (-0.00000199 * pow(ft, 2) * pow(Humid,2));
  return convert_fahrenheit_to_Celsius(hI);
}
//HI= c1+c2T+c3R+c4TR+c5T2+c6R2+c7T2R+c8TR2+c9T2R2
bool isNumber(double number){       
        char item[20];
        snprintf(item, sizeof(item), "%f\n", number);
        if( isdigit(item[0]) )
          return true;
        return false; 
} 
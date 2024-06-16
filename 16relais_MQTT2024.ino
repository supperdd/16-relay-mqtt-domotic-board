//## 2024 modif:
//## differentier pub et sub
//## commande d'etat
#include <Ethernet.h>// Ethernet.h library
uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x83};
#include <PubSubClient.h>
//#include <Wire.h>
#include "auth.h"
String ip = "192.168.0.83";
int inPin = 49;
int reading;
int previous = LOW;
unsigned long time = 0;
long debounce = 200;
int state = HIGH;
int mqttRetryAttempt = 0;
char messageBuffer[100];
char topicBuffer[100];
char clientBuffer[50];
long lastActivityTime   = 0;
                      
EthernetClient ethClient;
PubSubClient mqttClient;

long previousMillis;

/*Relay pins*/
const char r1 = 22;
const char r2 = 23;
const char r3 = 24;
const char r4 = 25;
const char r5 = 26;
const char r6 = 27;
const char r7 = 28;
const char r8 = 29;
const char r9 = 30;
const char r10 = 31;
const char r11 = 32;
const char r12 = 33;
const char r13 = 34;
const char r14 = 35;
const char r15 = 36;
const char r16 = 37;


/* ************************************************************************************* */
/* Button setup */
static byte lastButtonState[16] = {   0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0
                                          };


static byte relayArray[16]      =  { 22, 23, 24, 25, 26, 27, 28,
                                     29, 30, 31, 32, 33, 34, 35,
                                     36, 37
                                   };



byte lastButtonPressed         = 0;
#define DEBOUNCE_DELAY 50
/* ************************************************************************************* */

/* topics */  //    atmega/RB/r1/r*
                 //atmega=type de carte
                 //RB= carte relay + button
                 //r1= carte 1
                 //sub= souscriptions
                 //pub= publications
                 //r* = numero du relais
                 //PT= publish PT
#define R1_PT "atmega/RB/r83/pub/r1"                       ////////////////////////////////////topics de sortie
#define R2_PT "atmega/RB/r83/pub/r2"
#define R3_PT "atmega/RB/r83/pub/r3"
#define R4_PT "atmega/RB/r83/pub/r4"
#define R5_PT "atmega/RB/r83/pub/r5"
#define R6_PT "atmega/RB/r83/pub/r6"
#define R7_PT "atmega/RB/r83/pub/r7"
#define R8_PT "atmega/RB/r83/pub/r8"
#define R9_PT "atmega/RB/r83/pub/r9"
#define R10_PT "atmega/RB/r83/pub/r10"
#define R11_PT "atmega/RB/r83/pub/r11"
#define R12_PT "atmega/RB/r83/pub/r12"
#define R13_PT "atmega/RB/r83/pub/r13"
#define R14_PT "atmega/RB/r83/pub/r14"
#define R15_PT "atmega/RB/r83/pub/r15"
#define R16_PT "atmega/RB/r83/pub/r16"                 ///////////////////////////////////////////

#define R1_ST "atmega/RB/r83/sub/r1"                       ////////////////////////////////////topics d entrees
#define R2_ST "atmega/RB/r83/sub/r2"
#define R3_ST "atmega/RB/r83/sub/r3"
#define R4_ST "atmega/RB/r83/sub/r4"
#define R5_ST "atmega/RB/r83/sub/r5"
#define R6_ST "atmega/RB/r83/sub/r6"
#define R7_ST "atmega/RB/r83/sub/r7"
#define R8_ST "atmega/RB/r83/sub/r8"
#define R9_ST "atmega/RB/r83/sub/r9"
#define R10_ST "atmega/RB/r83/sub/r10"
#define R11_ST "atmega/RB/r83/sub/r11"
#define R12_ST "atmega/RB/r83/sub/r12"
#define R13_ST "atmega/RB/r83/sub/r13"
#define R14_ST "atmega/RB/r83/sub/r14"
#define R15_ST "atmega/RB/r83/sub/r15"
#define R16_ST "atmega/RB/r83/sub/r16"



String topicpubArray[16]     = {  "atmega/RB/r83/pub/r1", "atmega/RB/r83/pub/r2", "atmega/RB/r83/pub/r3", "atmega/RB/r83/pub/r4",
                               "atmega/RB/r83/pub/r5", "atmega/RB/r83/pub/r6", "atmega/RB/r83/pub/r7", "atmega/RB/r83/pub/r8",
                               "atmega/RB/r83/pub/r9", "atmega/RB/r83/pub/r10", "atmega/RB/r83/pub/r11", "atmega/RB/r83/pub/r12",
                               "atmega/RB/r83/pub/r13", "atmega/RB/r83/pub/r14", "atmega/RB/r83/pub/r15", "atmega/RB/r83/pub/r16" 
                            }; 

String topicsubArray[16]     = {  "atmega/RB/r83/sub/r1", "atmega/RB/r83/sub/r2", "atmega/RB/r83/sub/r3", "atmega/RB/r83/sub/r4",
                               "atmega/RB/r83/sub/r5", "atmega/RB/r83/ub/r6", "atmega/RB/r83/sub/r7", "atmega/RB/r83/sub/r8",
                               "atmega/RB/r83/sub/r9", "atmega/RB/r83/sub/r10", "atmega/RB/r83/sub/r11", "atmega/RB/r83/sub/r12",
                               "atmega/RB/r83/sub/r13", "atmega/RB/r83/sub/r14", "atmega/RB/r83/sub/r15", "atmega/RB/r83/sub/r16" 
                            }; 
                            
   
static byte buttonArray[16]     = {   2,  3,  5,  6, 
                                      7,  8,  9, 38,
                                     39, 40, 41, 42,
                                     43, 44, 45, 46, }; 
                                  
byte stateArray[16]     = {  0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0, 
                             };                             

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.print(topic);
  Serial.println("");

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  if (strcmp(topic,R1_ST)==0) {                 //si le topic suivit est 
    if ((char)payload[0] == '1') {              //à 1
      digitalWrite(r1, HIGH);                   //la sortie est deffinie a 1
      stateArray[0] = 1;
    } else {
      digitalWrite(r1, LOW);                    
      stateArray[0] = 0;
    }
  } else if (strcmp(topic,R2_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r2, HIGH);
      stateArray[1] = 1;
    } else {
      digitalWrite(r2, LOW);
      stateArray[1] = 0;
    }
  } else if (strcmp(topic,R3_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r3, HIGH);
      stateArray[2] = 1;
    } else {
      digitalWrite(r3, LOW);
      stateArray[2] = 0;
    }
  } else if (strcmp(topic,R4_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r4, HIGH);
      stateArray[3] = 1;
    } else {
      digitalWrite(r4, LOW);
      stateArray[3] = 0;
    }
  } else if (strcmp(topic,R5_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r5, HIGH);
      stateArray[4] = 1;
    } else {
      digitalWrite(r5, LOW);
      stateArray[4] = 0;
    }
  } else if (strcmp(topic,R6_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r6, HIGH);
      stateArray[5] = 1;
    } else {
      digitalWrite(r6, LOW);
      stateArray[5] = 0;
    }
  } else if (strcmp(topic,R7_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r7, HIGH);
      stateArray[6] = 1;
    } else {
      digitalWrite(r7, LOW);
      stateArray[6] = 0;
    }
  } else if (strcmp(topic,R8_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r8, HIGH);
      stateArray[7] = 1;
    } else {
      digitalWrite(r8, LOW);
      stateArray[7] = 0;
    }
  } else if (strcmp(topic,R9_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r9, HIGH);
      stateArray[8] = 1;
    } else {
      digitalWrite(r9, LOW);
      stateArray[8] = 0;
    }
  } else if (strcmp(topic,R10_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r10, HIGH);
      stateArray[9] = 1;
    } else {
      digitalWrite(r10, LOW);
      stateArray[9] = 0;
    }
  } else if (strcmp(topic,R11_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r11, HIGH);
      stateArray[10] = 1;
    } else {
      digitalWrite(r11, LOW);
      stateArray[10] = 0;
    }
  } else if (strcmp(topic,R12_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r12, HIGH);
      stateArray[11] = 1;
    } else {
      digitalWrite(r12, LOW);
      stateArray[11] = 0;
    }
  } else if (strcmp(topic,R13_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r13, HIGH);
      stateArray[12] = 1;
    } else {
      digitalWrite(r13, LOW);
      stateArray[12] = 0;
    }
  } else if (strcmp(topic,R14_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r14, HIGH);
      stateArray[13] = 1;
    } else {
      digitalWrite(r14, LOW);
      stateArray[13] = 0;
    }
  } else if (strcmp(topic,R15_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r15, HIGH);
      stateArray[14] = 1;
    } else {
      digitalWrite(r15, LOW);
      stateArray[14] = 0;
    }
  } else if (strcmp(topic,R16_ST)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r16, HIGH);
      stateArray[15] = 1;
    } else {
      digitalWrite(r16, LOW);
      stateArray[15] = 0;
    }
  }
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!mqttClient.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "DEDE_2560_ETH_I/O";
    /* connect now */
    if (mqttClient.connect(clientId.c_str(), serverUsername.c_str(), serverPassword.c_str())) {
      Serial.println("connected");
      /* subscribe topic */
      mqttClient.subscribe(R1_ST);
      mqttClient.subscribe(R2_ST);
      mqttClient.subscribe(R3_ST);
      mqttClient.subscribe(R4_ST);
      mqttClient.subscribe(R5_ST);
      mqttClient.subscribe(R6_ST);
      mqttClient.subscribe(R7_ST);
      mqttClient.subscribe(R8_ST);
      mqttClient.subscribe(R9_ST);
      mqttClient.subscribe(R10_ST);
      mqttClient.subscribe(R11_ST);
      mqttClient.subscribe(R12_ST);
      mqttClient.subscribe(R13_ST);
      mqttClient.subscribe(R14_ST);
      mqttClient.subscribe(R15_ST);
      mqttClient.subscribe(R16_ST);
      
    } else {
      Serial.print("failed, status code =");
      Serial.print(mqttClient.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
      mqttRetryAttempt++;
    }
  }
}

void setup() {
 Serial.begin(250000);
  while (!Serial) {};
  Serial.println(F("Carte IO 83"));
  Serial.println();

  // setup ethernet communication using DHCP
  if (Ethernet.begin(mac) == 0) {
    //Serial.println(F("Unable to configure Ethernet using DHCP"));
    for (;;);
  }

  Serial.println(F("Ethernet configured via DHCP"));
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println();
 //convert ip Array into String
  ip = String (Ethernet.localIP()[0]);
  ip = ip + ".";
  ip = ip + String (Ethernet.localIP()[1]);
  ip = ip + ".";
  ip = ip + String (Ethernet.localIP()[2]);
  ip = ip + ".";
  ip = ip + String (Ethernet.localIP()[3]);
  //Serial.println(ip);

  // setup mqtt client
  mqttClient.setClient(ethClient);

// Setting input pull-ups
  for( byte i = 0; i < 16; i++)
  {
    pinMode(buttonArray[i], INPUT_PULLUP);
  }

// Setting relay outputs
  for( byte i = 0; i < 16; i++)
  {
    pinMode(relayArray[i], OUTPUT);
  }  

  /* configure the MQTT server with IPaddress and port */
  mqttClient.setServer(serverHostname, 1883);
  /* this receivedCallback function will be invoked
    when client received subscribed topic */
  mqttClient.setCallback(receivedCallback);
  Serial.println("Everything Setup");
}

void loop() {
 if (!mqttClient.connected()) {
    mqttconnect();
  }
  mqttClient.loop();

  byte i;
  for( i = 0; i < 15; i++) {      //15
    processButtonDigital( i );
  }
}

void processButtonDigital( byte buttonId )
{
    int sensorReading = digitalRead( buttonArray[buttonId] );
    //Serial.print(buttonId, DEC);
    //Serial.print(": ");
    //Serial.println(sensorReading, DEC);
    
    if( sensorReading == 0 )  // Input pulled low to GND. Button pressed.
    {
      Serial.println( "Button pressed" );
      if( lastButtonState[buttonId] == 0 )   // The button was previously un-pressed
      {
        if((millis() - lastActivityTime) > DEBOUNCE_DELAY)  // Proceed if we haven't seen a recent event on this button
        {
          lastActivityTime = millis();
    
          lastButtonPressed = buttonId;
          Serial.print( "transition on ");
          Serial.print( buttonId, DEC );
          Serial.print(" (input ");
          Serial.print( buttonArray[buttonId] );
          Serial.println(")");
          //activer la sorie correspondante au bp presser
          
        
 //         String messageString = String(buttonId + 1);
          String messageString = String(stateArray[buttonId] == 0 ? 1 : 0);
          messageString.toCharArray(messageBuffer, messageString.length()+1);
        
//          String topicString = "device/" + String(panelId) + "/button";
          String topicString = String(topicpubArray[buttonId]);
          topicString.toCharArray(topicBuffer, topicString.length()+1);
          
          mqttClient.publish(topicBuffer, messageBuffer);
        
//          mqttClient.publish("buttons", messageBuffer);
//          mqttClient.publish(topicString, messageString);
        }
      } else {
        // Transition off
        //digitalWrite(statusArray[buttonId-1], LOW);
        //digitalWrite(13, LOW);
      }
      lastButtonState[buttonId] = 1;
    }
    else {
      lastButtonState[buttonId] = 0;
    }
}

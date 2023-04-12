// INCLUDE LIBRARIES FOR DISPLAY
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>


unsigned long last_run=0;
int res = 0;

/////////////////////////////////////
// DEFINED PINS FOR ROTARTY ENCODER//
/////////////////////////////////////
#define CLK  3 // digital 3 can be used as interrupt on UNO
#define DT  5
#define SW  2 // digital 2 can be used as interrupt on UNO

// STATE MANAGEMENT FOR ROTARTY ENCODER
int currentStateCLK;
int counter=0;
int oldCounter=0;
String currentDir="";
int buttonState = 0;
unsigned long lastButtonInterrupt = 0;

/////////////////////////////////////
///////// END ROTARY ENCODER ////////
/////////////////////////////////////

/////////////////////////////////////
////// BEGIN DISPLAY VARIABLES //////
/////////////////////////////////////

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
//for atmega328p 3.3v
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10
#define MAX_DEVICES 4
MD_Parola matrixDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String formattedTime = "00 :00";
String finalStr = "-- : --";
/////////////////////////////////////
////// BEGIN DISPLAY VARIABLES //////
/////////////////////////////////////

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(CLK), handleEncoder, LOW);
  attachInterrupt(digitalPinToInterrupt(SW), handleButton,  FALLING);
  pinMode(SW, INPUT_PULLUP);
  pinMode(DT, INPUT);
  currentStateCLK = digitalRead(CLK);
}

void handleButton() {
  if(digitalRead(SW) == LOW) {
    if(millis() - lastButtonInterrupt > 30) {
      Serial.println("Button has really been pressed");
    }
  }
  lastButtonInterrupt = millis();
}


void handleEncoder(){

  if (millis()-last_run>5){
    // Read the current state of CLK 
    currentStateCLK = digitalRead(CLK);

    // If the DT state is different than the CLK then encoder is rotating 
    // Counter clockwise so decrement
    if(digitalRead(DT) != currentStateCLK) {
      counter --;
      currentDir = "Counter clockwise";
    } else {
      // Encoder is rotating Clockwise so increment
      counter ++;
      currentDir = "Clockwise";
    }
      Serial.print("Rotating: | ");
      Serial.print(currentDir);
      Serial.print("Counter: | ");
      Serial.println(counter);
  }

   last_run=millis();
}

void loop() {
  if(counter != oldCounter) {
    Serial.print("counter : ");
    Serial.print(counter);
    Serial.print(" direction : ");
    Serial.println(currentDir);  
   
  }
 oldCounter = counter;
}
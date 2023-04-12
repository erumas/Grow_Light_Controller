// INCLUDE LIBRARIES FOR DISPLAY
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>


unsigned long last_run=0;
int res = 0;
/*************************
** BEGIN ROTARY ENCODER **
*************************/
#define CLK  3 // digital 3 can be used as interrupt on UNO
#define DT  5
#define SW  2 // digital 2 can be used as interrupt on UNO

// STATE MANAGEMENT FOR ROTARTY ENCODER
int currentStateCLK;
int currentValue=0;
int lastValue=0;
String currentDir="";
int buttonState = 0;
unsigned long lastButtonInterrupt = 0;
/***********************
** END ROTARY ENCODER **
************************/

/***********************
****  BEGIN DISPLAY ****
************************/
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
//for atmega328p 3.3v
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10
#define MAX_DEVICES 4
MD_Parola matrixDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String formattedTime = "00 :00";
String timeStr = "-- : --";
/***********************
*****  END DISPLAY *****
************************/

/****************************
**** BEGIN CLOCK & TIMER ****
****************************/
int hourOfDay = 0;
int minOfDay = 0;
String hourStr;
String minStr;
bool hasHourBeenSet = false;
bool hasMinBeenSet = false;
bool displayChange = true;
/****************************
***** END CLOCK & TIMER *****
****************************/

void setup() {
  Serial.begin(9600);
  // Setup interrupts and pins for rotary encoder
  attachInterrupt(digitalPinToInterrupt(CLK), handleEncoder, LOW);
  attachInterrupt(digitalPinToInterrupt(SW), handleButton,  FALLING);
  pinMode(SW, INPUT_PULLUP);
  pinMode(DT, INPUT);
  currentStateCLK = digitalRead(CLK);

  // initialize display
  matrixDisplay.begin();
  matrixDisplay.setIntensity(1);

  // welcome message
  matrixDisplay.setTextAlignment(PA_CENTER);
  matrixDisplay.print("Light :)");
  delay(2000);

  matrixDisplay.displayClear();
  matrixDisplay.displayScroll("Staring up controller!", PA_CENTER, PA_SCROLL_LEFT, 100);

  updateDisplay(formattedTime);
}

void updateDisplay (String toDisplay) {
  matrixDisplay.displayClear();
  matrixDisplay.setTextAlignment(PA_CENTER);
  matrixDisplay.print(toDisplay);
}

void handleButton() {
  if(digitalRead(SW) == LOW) {
    if(millis() - lastButtonInterrupt > 30) {
      Serial.println("Button has really been pressed");
      if(!hasHourBeenSet) {
        hasHourBeenSet = true;
      }
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
      currentValue --;
      currentDir = "Counter clockwise";
      
    } else {
      // Encoder is rotating Clockwise so increment
      currentValue ++;
      
      currentDir = "Clockwise";
    }
      Serial.print("Rotating: | ");
      Serial.print(currentDir);
      Serial.print("CurrentValue: | ");
      Serial.println(currentValue);
  }

  if(!hasHourBeenSet && currentValue != lastValue ) {
    // 1. set time of day : hours first
    if(currentValue > lastValue) {
      hourOfDay ++;
    } else {
      hourOfDay --;
    } 

    if(hourOfDay > 23) {
      hourOfDay = 0;
    } else if (hourOfDay < 0) {
      hourOfDay = 23;
    }
  } else if (!hasMinBeenSet && hasHourBeenSet && currentValue != lastValue) {
    // 1. set time of day : minutes
    if(currentValue > lastValue) {
      minOfDay ++;
    } else {
      minOfDay --;
    } 

    if(minOfDay > 59) {
      minOfDay = 0;
    } else if (minOfDay < 0) {
      minOfDay = 59;
    }
  }
  
  if(hourOfDay < 10) {
    hourStr = '0' + String(hourOfDay);
  } else {
    hourStr = String(hourOfDay);
  }
  
  if(minOfDay < 10) {
    minStr = '0' + String(minOfDay);
  } else {
    minStr = String(minOfDay);
  }
  
  
  timeStr =  hourStr + ':' + minStr;  
  last_run=millis();
  lastValue = currentValue;
  displayChange = true;
}

void loop() {
  
  if(displayChange) {
    Serial.print("currentValue : ");
    Serial.print(currentValue);
    Serial.print(" direction : ");
    Serial.println(currentDir);  
    updateDisplay(timeStr);
    displayChange = false;
  }
 
}
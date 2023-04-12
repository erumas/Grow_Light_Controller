// INCLUDE LIBRARIES FOR DISPLAY
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>


#include <TimeLib.h>
#include <TimeAlarms.h>
unsigned long last_run=0;
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

String timeStr = "-- : --";
/***********************
*****  END DISPLAY *****
************************/

/****************************
**** BEGIN CLOCK & TIMER ****
****************************/
String hourStr;
String minStr;
bool hasHourBeenSet = false;
bool hasMinBeenSet = false;
bool hasTimeBeenConfirmed = false;
bool hasSessionLengthBeenSet = false;
bool displayChange = true;
bool hasAllBeenSet = false;
int tempHour = 0;
int tempMin = 0;

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

  //set time
  setTime(8,29,0,1,1,11); 
   
  // setTime(8,29,0,1,1,11); // set time to Saturday 8:29:00am Jan 1 2011

  // create the alarms, to trigger at specific times
  Alarm.alarmRepeat(8,30,0, turnOnLights);  // 8:30am every day

  updateDisplay(timeStr);
  // Alarm.timerRepeat(15, Repeats);   
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
        return;
      }

      if(!hasMinBeenSet) {
        hasMinBeenSet = true;
        return;
      }
      
      // Everything has been confirmed, 
      // save values and reset to move onto next step
      if(!hasTimeBeenConfirmed) {
        setTime(tempHour,tempMin,0,1,1,11);         
        hasTimeBeenConfirmed = true;
        resetTimeVals();
        formatTime();
        displayChange = true;
        return;
      } 

      if(!hasSessionLengthBeenSet) {
        // set start time/
        Alarm.alarmRepeat(tempHour, tempMin, 0, turnOnLights);
        hasSessionLengthBeenSet = true;
        
        resetTimeVals();
        formatTime();
        displayChange = true;
        return;
      }

      if(!hasAllBeenSet) {
        hasTimeBeenConfirmed = true;
        startHour = tempHour;
        startMin = tempMin;
        hasAllBeenSet = true;
        displayChange = true;
        updateDisplay("Ready");
        return;
      }
      return;
    }
  }
  
  lastButtonInterrupt = millis();
}

void turnOnLights() {
  Serial.println("turning on lights");
}

void turnOffLights() {
  Serial.println("turning off lights");
}

void resetTimeVals() {
  tempHour = 0;
  tempMin = 0;
  hasHourBeenSet = false;
  hasMinBeenSet = false;
}

void handleEncoder(){
  if (millis()-last_run>5){
    // Read the current state of CLK 
    currentStateCLK = digitalRead(CLK);

    // If the DT state is different than the CLK then encoder is rotating 
    // Counter clockwise so decrement
    if(digitalRead(DT) != currentStateCLK) {
      currentValue --;
    } else {
      // Encoder is rotating Clockwise so increment
      currentValue ++;
    }
  }

  if(!hasHourBeenSet && currentValue != lastValue ) {
    // 1. set time of day : hours first
    if(currentValue > lastValue) {
      tempHour ++;
    } else {
      tempHour --;
    } 

    if(tempHour > 23) {
      tempHour = 0;
    } else if (tempHour < 0) {
      tempHour = 23;
    }
  } else if (!hasMinBeenSet && hasHourBeenSet && currentValue != lastValue) {
    // 2. set time of day : minutes
    if(currentValue > lastValue) {
      tempMin ++;
    } else {
      tempMin --;
    } 

    if(tempMin > 59) {
      tempMin = 0;
    } else if (tempMin < 0) {
      tempMin = 59;
    }
  }
  formatTime();
}

void formatTime() {
  if(tempHour < 10) {
    hourStr = '0' + String(tempHour);
  } else {
    hourStr = String(tempHour);
  }
  
  if(tempMin < 10) {
    minStr = '0' + String(tempMin);
  } else {
    minStr = String(tempMin);
  }

  timeStr =  hourStr + ':' + minStr;
  

  last_run=millis();
  lastValue = currentValue;
  displayChange = true;
}

void loop() {
  // Serial.println("in loop");
  // Serial.print(hour());
  // Serial.println(minute());
  
  String clock= String(hour()) + ":" + String(minute());
  if(displayChange) {
    // Serial.print("currentValue : ");
    // Serial.print(currentValue);
    // Serial.print(" direction : ");
    // Serial.println(currentDir);
    if(hasAllBeenSet){
      
      updateDisplay(clock);
    } else {
      updateDisplay(timeStr);
    }

    displayChange = false;
  }
  Serial.print("time | ");
  Serial.print(hour() + ":");
  Serial.println(minute());
  updateDisplay(clock);
  // This has to be called, the alarms are triggered in the delay
  Alarm.delay(500); // wait one second between clock display
}
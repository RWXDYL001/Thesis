/*
Created By: Dylan Raw - RWXDYL001
For: University Of Cape Town - Final Thesis
Date: 25/10/2023
*/

//Included Libraries below
#include <SPI.h>
#include <SD.h>
#include <ADC.h>
#include <ADC_util.h>

//Initiated Variables below
int t;          //Time variable
int value;      //Reading variable
int check = 0;  //Flag for if readings have been taken
int count = 0;  //Count of the number of samples taken

int file = 0;          //File naming identifier (Int value)
char fileString[30];   //File name for data file
char fileString2[30];  //File name for sampling ststistics

int inputPin = 4;  //ADC pin number

int SwitchState = 0;                 //Int for Switch state (1 or 0) initialiased at 0
int lastSwitchState = 0;             //Int for the last Switchstate (1 or 0) initialised at 0
int currentSwitchState = 0;          //Int for the current Switch stste (1 or 0) initialised at 0
unsigned long lastDebounceTime = 0;  //Long value for the last debounce time
unsigned long debounceDelay = 5;     //Int value for the debounce delay in ms

ADC *adc = new ADC();    // ADC object
const int readPin = A4;  // ADC0 pin number
File myFile;             //Initialising the file object myFile

//Setup Function to initalise the system
void setup() {
  Serial.begin(9600);        //Initialises the Serial Port
  pinMode(inputPin, INPUT);  //Sets the Pin to Input mode

  if (!SD.begin(10)) {  //Initalises the SD card
    while (1)
      ;
  }

  adc->adc0->setAveraging(32);                                     // sets number of averages
  adc->adc0->setResolution(12);                                    // sets bits of resolution
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);  // changes the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);     // changes the sampling speed

  filenaming();  //Identifies and sets the next file names

  delay(500);  //Delay after setup loop is complete
}

//Main loop function
void loop() {
  if (checkswitch() == HIGH) {                   //If check switch is high begin sampling i.e. Switch is On
    if (check == 0) {                            //Checks the sample flag to determine if a sample has been recorded
      myFile = SD.open(fileString, FILE_WRITE);  //Opens the File on the SD Card in Write mode
      t = millis();                              // Start time of samples taken
    }
    value = adc->adc0->analogRead(readPin);                       //Reads an ADC value from the pin
    myFile.println(value * 3.3 / adc->adc0->getMaxValue(), DEC);  //Converts the ADC value to a voltage and prints it to the file
    count++;                                                      //Increments the sample counter
    check = 1;                                                    // Flag to indicate samples have been read
  }
  while (checkswitch() == LOW) {                  //State not sampling i.e. Switch is Off
    if (check == 1) {                             //Checks sample flag to determine if samples have been taken
      myFile.close();                             //Closes Data file
      myFile = SD.open(fileString2, FILE_WRITE);  //Opens the Sampling statistics file in write mode
      int tm = millis() - t;                      //Calculates the time taken for all the samples in ms
      float ts = tm / 1000;                       //Converts the time into seconds
      float sr = count / ts;                      //Calculates the sample rate of the system in Hz
      myFile.print("Time Taken [s]: ");           //Prints the time taken in seconds to the statistics file
      myFile.println(ts);
      myFile.print("Samples Taken: ");      //Prints the Sample Count to the statistics file
      myFile.println(count);
      myFile.print("Sample Rate [Hz]: ");   //Prints the Sample Rate in Hz to the statistics file
      myFile.println(sr);
      myFile.close();  //Closes the ststistics file
      delay(100);      //Delay after closing the file for protection
      filenaming();    //Determines and sets the new file names
      delay(100);      //Delay for protection
      count = 0;       //Resets the Sample count to 0
      t = 0;           //Resets the time taken to 0
    }
    check = 0;  //Set the sampling flag to 0
  }
}

/*
Filenaming Function
Checks through the SD card to determine in a file name is present
If the name is present it increments the file indicator and checks again.
once a file name is found to not be on the SD card the function will
set the current file name to this value
*/
void filenaming() {
  sprintf(fileString, "testdata%02d.txt", file);   //Sets file name to check
  sprintf(fileString2, "testtime%02d.txt", file);  //Sets statistics file name
  do {
    file = file + 1;                                 //Increments File indicator
    sprintf(fileString, "testdata%02d.txt", file);   //Sets New file name
    sprintf(fileString2, "testtime%02d.txt", file);  //Sets New statistics File name
  } while (SD.exists(fileString));                   //Checks if new file name is found on the SD card
}


/*
Check Switch Function
This function is used to debounce the mechanical switch in the circuit
This is used to prevent rapin switching when changing the switchs state
*/
int checkswitch() {
  currentSwitchState = digitalRead(inputPin);     //Sets the current switch state
  if (currentSwitchState != lastSwitchState) {    //Checks is the current switch state is different from the last switch state
    lastDebounceTime = millis();                  //Starts the debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {    //Checks if the time elapsed exceeds the debounce delay
    if (currentSwitchState != SwitchState) {              //Checks if current switch state different from the set switch state
      SwitchState = currentSwitchState;                   //Sets the switch state to the new value
      return SwitchState;                                 //Returns the new switch state
    }
  }
  lastSwitchState = currentSwitchState;                   //Sets last switch state to current switch state
  return lastSwitchState;                                 //Returns last switch state
}

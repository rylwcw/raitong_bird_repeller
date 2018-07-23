/*******************************************************
    Author: Benjamin Low
    Date: Jul 2018
    Title: Raitong bird repeller
    Description: Programming logic for an mp3 player connected to a megaphone

    Author for MP_Module library:  Continental Electronic Enterprise
    Version     V1.0
    Copyright www.continentalee.com.sg
    All right reserved.
    Source: https://github.com/continental-electronics/MP3-Module
    This library works for MP3 module sold @ www.continentalee.com.sg.

    Connections:
        MP3 player:
            wemos D5 (GPIO14) -> MP3 rx
            wemos D6 (GPIO12) -> MP3 tx
            5V -> MP3 Vcc
            GND -> mp3 GND

        buttons:
            wemos D3 (pull-up) - play/pause
            wemos D4 (pull-up) - prev
            wemos D8 (pull-down) - next
            wemos D7 (pull-up) - auto or manual mode selection

        OLED shield:
            wemos D0 -> OLED D0
            wemos D1 -> OLED D1
            wemos D2 -> OLED D2
            3.3V -> OLED 3.3V
            GND -> OLED GND

    Note:
        - Software serial pin definitions changed in the library:
            SoftwareSerial COM_SOFT(14, 12); //wemos rx, tx
        - Auto mode means automated script running for sound program.
          Manual mode means user manual operation using the buttons.
*******************************************************/

#include <MP3_Module.h>
#include <SoftwareSerial.h>
#include <Button.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MANUALMODE 0
#define AUTOMODE 1

//User settings
const unsigned long INTERVALLOWERBOUND = 5000; //lower bound for time interval delay between blasts
const unsigned long INTERVALUPPERBOUND = 10000; //upper bound
const int NUMOFSOUNDS = 15;//the number of sound samples in the SD card
const unsigned long SCREENIDLETIME = 600000; //idle duration before screen auto shutoff

//mp3 player
MP3 mp3;

//global variables
unsigned long lullInterval;//will be randomised between lower and upper bound
int selectedTrack;//can be randomised between 1 and NUMOFSOUNDS;
unsigned long timeLastPlayed;//time the last sound was played
bool isPlaying;
int operationMode = MANUALMODE;
unsigned long lastButtonPressedTime;
bool isScreenIdle;

//Buttons
Button playPauseButton = Button(D3, PULLUP);
Button prevButton = Button(D4, PULLUP);
Button nextButton = Button(D8, PULLDOWN);
Button modeButton = Button(D7, PULLUP);

//OLED 
#define OLED_RESET D0
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 48) //64x48 pixels
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() {
  Serial.begin(9600);
  mp3.begin(); // initialize mp3
  mp3.volume(30); //0-30, set max volume, does not matter since the 50W audio amplifier will amplify to max
  mp3.set_mode(2); //0 - ALL, 1 - FOLDER, 2 - ONE, 3 - RANDOM, 4 - ONE STOP
  Serial.println("mp3 player started");
  //  mp3.select_file(1); //this will auto play
  selectedTrack = 1;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Raitong\n Bird\n Repeller");
  display.display();
  delay(2000);
}

void loop() {
  if (operationMode == AUTOMODE) { 
    
    //your automated script here 
    if (millis() - timeLastPlayed > lullInterval && !isPlaying) {
      isPlaying = true;
      selectedTrack = random(1, NUMOFSOUNDS + 1); //min is inclusive, max is exclusive
      mp3.select_file(selectedTrack);
      lullInterval = random(INTERVALLOWERBOUND, INTERVALUPPERBOUND); //set the next random interval
      timeLastPlayed = millis();
      Serial.print("millis(): "); Serial.println(millis());
      Serial.print("file num playing: "); Serial.println(selectedTrack); Serial.println();
      mp3.play();
    } else {
      isPlaying = false;
    }
  } else { 
    
    //manual mode with button operation
    if (playPauseButton.uniquePress()) {
      lastButtonPressedTime = millis(); isScreenIdle = false;
      if (mp3.get_status() == 1) { //(0 - STOP, 1 - PLAY, 2 - PAUSE)
        mp3.pause();
        Serial.println("paused");
      } else {
        mp3.play();
        Serial.println("playing");
      }
    } else if (prevButton.uniquePress()) {
      Serial.println("prev button pressed");
      lastButtonPressedTime = millis(); isScreenIdle = false;
      selectedTrack -= 1;
      if (selectedTrack < 1) {
        selectedTrack = NUMOFSOUNDS;
      }
      Serial.print("file num playing: "); Serial.println(selectedTrack); Serial.println();
      mp3.select_file(selectedTrack);
    } else if (nextButton.uniquePress()) {
      lastButtonPressedTime = millis(); isScreenIdle = false;
      Serial.println("next button pressed");
      selectedTrack += 1;
      if (selectedTrack > NUMOFSOUNDS) {
        selectedTrack = 1;
      }
      Serial.print("file num playing: "); Serial.println(selectedTrack); Serial.println();
      mp3.select_file(selectedTrack);
    }
  }

  //check mode operation button
  if (modeButton.uniquePress()) {
    lastButtonPressedTime = millis(); isScreenIdle = false;
    Serial.println("mode button pressed");
    if (operationMode == AUTOMODE) operationMode = MANUALMODE;
    else operationMode = AUTOMODE;
    Serial.print("operation mode: "); Serial.println(operationMode);
  }

  //screen idle time check
  if (millis() - lastButtonPressedTime > SCREENIDLETIME) {
    display.clearDisplay();
    display.display();
    isScreenIdle = true;
  }

  //display something on screen
  if (!isScreenIdle) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    if (operationMode == MANUALMODE) display.println("Manual\n mode\n");
    else display.println("Auto mode\n");
    display.print("Track #"); display.print(selectedTrack);
    display.display();
  }
}



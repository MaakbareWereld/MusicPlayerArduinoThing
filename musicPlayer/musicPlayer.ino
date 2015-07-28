/*
This is a variant on the example for the Adafruit VS1053 Codec Breakout.

This is used to play a list of MP3's. I used this setup in a small music
machine that I build for my 2.5 year old.

Two pins (2 and 5) are connected to ground by simple pushbuttons (no
pull-up needed, since the INPUT_PULLUP pinmode is used). Pin 2 is a play
and stop button, pin 5 is a next song button. 

The files on the SD should be called 1.mp3, 2.mp3 etc. This program is not
sofisticated enough to scan for filenames. (Feel free to add this functio-
nality!)

Finally, the music will stop playing after ten minutes!

On this software, the Apache 2.0 license applies (http://www.apache.org/licenses/LICENSE-2.0)

*/


/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins. 
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

//pin for play and pause
#define playPin 2
//pin for next song
#define nextPin 5
//pin for checking if playing
#define ledPin 13

boolean nowPlaying = false;
boolean playStatus = false;
int currentSong = 1;
int heartbeat = 0;
char playFile[6];
unsigned long buttonPressedTime;
unsigned long sleepTime = 600000; //ten minutes of music and than sleep!
File currentFile;

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  //Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
  
void setup() {
 
  
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
  SD.begin(CARDCS);    // initialise the SD card
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(1,1);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  //set modes for play and next pin
  pinMode(playPin, INPUT_PULLUP);
  pinMode(nextPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  
  //start with Song number one:
  (String(currentSong) + ".mp3").toCharArray(playFile,6);
  
}

void loop() {
  //if play/pause is pushed
  if (digitalRead(playPin)==LOW){
    delay(100);
    playStatus = !playStatus;
    digitalWrite(ledPin,playStatus);
    Serial.println(playStatus);
    if (playStatus == true){
      musicPlayer.startPlayingFile(playFile);
      buttonPressedTime = millis();
    }
    else{
      musicPlayer.currentTrack.close();
      musicPlayer.stopPlaying();
    }
    delay(400);
  }
  
  //if "next song" is pushed
  if (digitalRead(nextPin) == LOW){
    delay(100);
    currentSong++;
    (String(currentSong) + ".mp3").toCharArray(playFile,6);
    Serial.println(playFile);
    if (!SD.exists(playFile)){
      currentSong=1;
      (String(currentSong) + ".mp3").toCharArray(playFile,6);
      Serial.print("file did not exist, restarting with file: ");
      Serial.println(playFile);
    }
    if (playStatus == true){
      musicPlayer.currentTrack.close();
      musicPlayer.stopPlaying();
      musicPlayer.startPlayingFile(playFile);
    }
    delay(400);
  }
  
  //check if playing for to long
  unsigned int timePlaying = ((unsigned int) millis()) - buttonPressedTime;
  if ((musicPlayer.playingMusic == true) && (timePlaying > sleepTime)){
    playStatus = false;
    musicPlayer.currentTrack.close();
    musicPlayer.stopPlaying();
    delay(500);
    //and go the f#ck to sleep.
  }
  
  if ((timePlaying % 1000) < 10){
    Serial.println(heartbeat++);
    delay(10);
  }
  delay(1); 
}


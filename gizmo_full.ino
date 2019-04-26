/*
   Colin the cyclist gizmo script
   Author: Jordan Kotler

   Credit to learnelectronics for spectrum analyser OLED code

   Fix_FFT library available @ https://github.com/kosme/arduinoFFT
*/


#include <fix_fft.h>                                  //library to perfom the Fast Fourier Transform
#include <SPI.h>                                      //SPI library
#include <Wire.h>                                     //I2C library for OLED
#include <Adafruit_GFX.h>                             //graphics library for OLED
#include <Adafruit_SSD1306.h>                         //OLED driver
#define SCREEN_WIDTH 128                              //OLED display width, in pixels
#define SCREEN_HEIGHT 64                              //OLED display height, in pixels
#define OLED_RESET 2                                  //OLED reset, not hooked up but required by library
#define MOTOR_AIN1 9                                  //set PWM pins on arduino
#define MOTOR_AIN2 10
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);                 //declare instance of OLED library called display

#include "Deque.h"
Deque<int> scoreHistory;                              //use Deque class for moving average, pop head push tail

char im[128], re[128];                                //variables for the FFT
char x = 0, ylim = 60;                                //variables for drawing the graphics
int i = 0, val;                                       //initialising loop counters and other variables
int j;

int scorehi = 0;                                      //initialises scorehi for screen display
int scorelo = 0;                                      //sets scorelo, necessary to allow scorehi to be changed if its > scorelo
int dynamicScorehi = 0;                               //allows for a dynamic variable to be fed to motor

int minInput = 250;         //microphone min value isnt 0 due to noise and hardware discrepancy, depending on room volume this must be edited
int maxInput = 550;         

int velocity = 0;                 //initialise value to be displayed on end screen as highscore

const int buttonPin = 7;                             //set button to pin 7
int buttonState;
int STATE = 0;
int windowSize = 750/75;                              // 750 millisec for avg to reach highscore, delay in motor speed update essentially


void setup()
{
  Serial.begin(9600);                                 //serial comms for debuging
  pinMode(MOTOR_AIN1, OUTPUT);                        //set motor pin
  pinMode(MOTOR_AIN2, OUTPUT);                        //set motor pin
  pinMode(buttonPin, INPUT);                          //set button to A0
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);          //begin OLED @ hex addy 0x3D
  display.setTextSize(1.5);                           //set OLED text size to 1 (1-6)
  display.setTextColor(WHITE);                        //set text color to white
  initMotor();
  bool result = scoreHistory.setLimit(windowSize);    //size of the (sampling) window
  Serial.print("Limit was set: ");
  Serial.println(result);

};

void drawWelcome() {            // welcome screen display
  display.setCursor(28, 0);
  display.print("Hello there!");
  display.setCursor(5, 15);
  display.print("Cheer on the cyclist");
  display.setCursor(5, 25);
  display.print("as loud as you can!!");
  display.setCursor(17, 40);
  display.print("Press the green");
  display.setCursor(17, 50);
  display.print("button to start.");
}

void drawTitle() {                                //screen title for 'playing' screen
  display.setCursor(25, 0);                       //set cursor to top of screen
  display.print("SHOUT LOUDER!");                 //print title
}

void drawEnd() {                        //end screen, display depends on your performance in the game
  if ((scorehi-minInput)>220) {         //if you shout loud enough then OLED displays "Colin finished his race!!! Your high score is X mph" at the end
  display.setCursor(10, 5);
  display.print("Colin finished his");
  display.setCursor(50, 15);
  display.print("race!!");
  display.setCursor(10, 35);
  display.print("Your high score is:");
  display.setCursor(52, 50);
  display.print(velocity);
  display.setCursor(67, 50);
  display.print("mph");
  }

  else{
  display.setCursor(3, 0);             //else, if you don't shout loud enough then OLED displays "Colin did not finish his race, you did not shout loud enough. Your highscore is X mph"
  display.print("Colin did not finish");
  display.setCursor(0, 10);
  display.print("his race, you did not");
  display.setCursor(8, 20);
  display.print("shout loud enough.");
  display.setCursor(10, 35);
  display.print("Your high score is:");
  display.setCursor(52, 50);
  display.print(velocity);
  display.setCursor(67, 50);
  display.print("mph");
  }
}

void takeSamples() {          //function takes sample from microphone using analogRead
  
  int mini = 1024, maxi = 0;                          //set minumum & maximum ADC values
  for (i = 0; i < 128; i++) {                         //take 128 samples
    val = analogRead(A1);                             //get audio from Analog 1
    re[i] = val / 4 - 128;                            //each element of array is val/4-128
    im[i] = 0;
  };
}

void updateMax() {                                    //updates the highscore shown on screen, also feeds new values to push to moving average
  scorelo = analogRead(A1);
  //scorelo = map(scorelo, minInput, maxInput, scoreMinConstraint, scoreMaxConstraint);
  //scorelo = constrain(scorelo, scoreMinConstraint, scoreMaxConstraint);

  if (scorelo > scorehi) {                            //updates on-screen highscore
    scorehi = scorelo;
  }
  if (scorelo > dynamicScorehi) {                     //updates dynamicScorehi which affects motor PWM
    dynamicScorehi = scorelo;
  }
  display.setCursor(7, 15);
  display.print("Your highscore=");
  display.print(scorehi-minInput);                    //minInput is value accounting for noise in the room, so that the highscore starts at 0 instead of some arbitrary level

  if (scoreHistory.count() == windowSize) scoreHistory.popHead();   //once set window size is full it will pop old value and push new value constantly, creating moving window
  scoreHistory.pushTail(dynamicScorehi);
}

int dat;
void drawGraph() {                                                     //draws spectrum analyses on screen as a bar graph, each bar is a frequency band
  for (j = 1; j < 64; j++) {                                           // In the current design, 60Hz and noise
    dat = sqrt(re[j] * re[j] + im[j] * im[j]);                         //filter out noise and hum
    display.drawLine(j * 2 + x, ylim, j * 2 + x, ylim - dat, WHITE);   // draw bar graphics for freqs above 500Hz to display
  }
}

void initMotor() {                  //initialise motor
  digitalWrite(MOTOR_AIN1, LOW);
  analogWrite(MOTOR_AIN2, 0);
}


int scorehiAverage() {
  int sha = 0;
  for (int i = 0; i < scoreHistory.count(); i++) {
    sha += scoreHistory[i];
  }

  int average = sha / scoreHistory.count();     //calculates the average of the window taken, dividing sum of values in the window by number of values in the window
  Serial.print(scorehi);                        //plots variables in serial plotter to get a visual indication of values changing over time
  Serial.print(",");
  Serial.print(dynamicScorehi);
  Serial.print(",");
  Serial.println(average);
  return average;
}

void motor() {
  int avgScorehi = scorehiAverage();      //creates variable equal to value returned from scorehiAverage() function
  digitalWrite(MOTOR_AIN1, LOW);
  int pwm = map(avgScorehi, minInput, maxInput, 0, 255);                  //map the average score to a pwm value
  analogWrite(MOTOR_AIN2, pwm);
}

void stopMotor() {
  digitalWrite(MOTOR_AIN1, LOW); //used to stop motor once game is complete
  analogWrite(MOTOR_AIN2, 0);
}

bool stateNotRun = true;
long stateStartMillis;      //long as this is the data type millis() returns
long testStartMillis;


void loop() {
  display.clearDisplay();
  buttonState = digitalRead(buttonPin);

  if (STATE == 0) // first state of the game, welcome screen until button is pressed
  {
    if (stateNotRun) {                  //always true at start of state as set outside void
      stateStartMillis = millis();      //allows for a timestamp to be created using millis()
      stateNotRun = false;              //if loop is now exited so that stateStartMillis is not reset
    }
    drawWelcome();
    if (buttonState == 1 && millis() - stateStartMillis >= 1000) { // will only skip to next state if the button is pressed and 1 second has passed to avoid button bounce/user error pressing button more than once
      STATE++;                                                     //increments state to move onto next screen
      stateNotRun = true;                                          //resets stateNotRun for next loop
    }
  }

  else if (STATE == 1) // second state of the game, when you are shouting to increase motor speed
  {
    if (stateNotRun) {                  //reset to true at end of last loop
      stateStartMillis = millis();      //allows for a new timestamp to be created using millis()
      stateNotRun = false;              //if loop is now exited so that stateStartMillis is not reset
    }

    drawTitle();                        //calls on all necessary functions to play game
    takeSamples();
    dynamicScorehi -= 5; // decreases motor speed if you dont consistently shout
    updateMax();
    motor();
    fix_fft(re, im, 7, 0);                        //perform the FFT on data, fft function is imported
    drawGraph();

    velocity = (scorehi / 10);                    //convert max amplitude score into some relatable speed that can be understood by user (50mph instead of 467)

    if ( millis() - stateStartMillis >= 20000 || (buttonState == 1 && millis() - stateStartMillis >= 4000) ) //users plays for 20secs, user can end game after 4 seconds if wanted, also useful for debugging to not waste 20 seconds everytime code is tested
    {
      STATE++ ;
      stateNotRun = true;
    }

  } // end state 1

  else if (STATE ==  2) // final state of the game, end screen displaying highscore
  {
    if (stateNotRun) {                //logic same as previous states
      stateStartMillis = millis();
      stateNotRun = false;
    }

    drawEnd();                    //display end screen
    stopMotor();                  //kills motor at the end otherwise it keeps running

    if ( millis() - stateStartMillis >= 10000 || (buttonState == 1 && millis() - stateStartMillis >= 2000) ) //returns to homescreen if 10 seconds passes after end of user's turn, or if the button is pressed
    {
      STATE = 0;                                      //returns to welcome screen
      stateNotRun = true;
      scorehi = 0;                                   //resets variables for next player so arduino doenst need to be hard reset and next user can play immediately
      scorelo = 0;                                  
    }
  }

  display.display();            //updates OLED
};

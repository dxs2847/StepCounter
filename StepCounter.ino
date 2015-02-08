/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

#include "Ladder.h"

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// Define the program states
#define STATE_SETUP    0x1
#define STATE_READY    0x2
#define STATE_RUNNING  0x3
#define STATE_JUMPING  0x4
#define STATE_COMPLETE 0x5

#define LCD_PADDING_ZERO '0'
#define LCD_PADDING_SPACE ' '
#define LCD_POSITION_RUNG 14
#define LCD_POSITION_SCR 8
#define LCD_POSITION_RIGHT 15
#define LCD_LINE_ZERO 0
#define LCD_LINE_ONE 1

#define MAX_JUMPS 7

int programState;
int displayState;
int rung;
uint8_t buttonState;

int currentSteps;
int currentJumps;
int switchState;
boolean justJumped;

void setup() {
	// Debugging output
	Serial.begin(9600);
	// set up the LCD's number of columns and rows: 
	lcd.begin(16, 2);

	pinMode(35, INPUT_PULLUP);
	programState = STATE_SETUP;
	displayState = 0;
	buttonState = 0;
	rung = 1;
	currentSteps = 0;
	currentJumps = 0;
	justJumped = false;
	switchState = HIGH;

	lcd.setBacklight(WHITE);
	updateDisplay();
}

void loop() 
{
	switch(programState){		
		case STATE_SETUP:
			processButtons();
			break;
		case STATE_READY:
			processSwitch();
			break;
		case STATE_RUNNING:
			processSwitch();
			if(currentSteps == introLadder[rung]){
				programState = STATE_COMPLETE;
			}else if(currentSteps != 0 && currentSteps % 75 == 0 && !justJumped){
				programState = STATE_JUMPING;
			}
		case STATE_JUMPING:
			processSwitch();
			if(currentJumps == MAX_JUMPS){
				currentJumps = 0;
				justJumped = true;
				programState = STATE_RUNNING;
			}
		case STATE_COMPLETE:
			break;
	}
	
	updateDisplay();
}

int newSwitchState;
int newSwitchState2;
void processSwitch(){
	// Read (and debounce) switch
	newSwitchState = digitalRead(35);
	delay(10);
	newSwitchState2 = digitalRead(35);
	
	if(newSwitchState == newSwitchState2 && switchState != newSwitchState){
		switchState = newSwitchState;
		
		if(switchState == LOW){
			switch(programState){
				case STATE_READY:
					programState = STATE_RUNNING;
					break;
				case STATE_RUNNING:
					justJumped = false;
					currentSteps++;
					displayCurrentSCR(LCD_LINE_ONE);
					break;
				case STATE_JUMPING:
					currentJumps++;
					displayJumps();
					break;
				default:
					break;
			}
		}
	}
}

void processButtons(){
	uint8_t buttons = lcd.readButtons();
	int oldRung = rung;

	
	if ((buttons & BUTTON_UP) && buttonState != BUTTON_UP) {
		buttonState = BUTTON_UP;
		rung++;
	}
	else if ((buttons & BUTTON_DOWN) && buttonState != BUTTON_DOWN) {
		buttonState = BUTTON_DOWN;
		rung--;
	}
	else if ((buttons & BUTTON_LEFT) && buttonState != BUTTON_LEFT) {
		buttonState = BUTTON_LEFT;
		rung = 1;
	}
	else if ((buttons & BUTTON_RIGHT) && buttonState != BUTTON_RIGHT) {
		buttonState = BUTTON_RIGHT;
		rung += 10;
	}
	else if ((buttons & BUTTON_SELECT) && buttonState != BUTTON_SELECT) {
		buttonState = BUTTON_SELECT;
		programState = STATE_READY;
	}
	
	if(buttons == 0) {
		buttonState = 0;
	}
	
	if(rung != oldRung)
	{
		if(rung > MAX_RUNG){
			rung = MAX_RUNG;
		}

		if(rung < MIN_RUNG){
			rung = MIN_RUNG;
		}
	
		displayRung();
		displaySelectedSCR(LCD_LINE_ONE);
	}
}

void displayNumber(int num, int width, int pos, int line, char padding){
	lcd.setCursor(pos, line);
	
	Serial.print("num: "); Serial.print(num); Serial.print(", width: "); Serial.print(width); Serial.print(", line: "); Serial.println(line);

	while(width > 0 && num < pow(10, --width)){
		lcd.print(padding);
		lcd.setCursor(++pos, line);
	}

	lcd.print(num, 10);
}

void displayRung(){
	displayNumber(rung, 2, LCD_POSITION_RUNG, LCD_LINE_ZERO, LCD_PADDING_ZERO);
}

void displaySelectedSCR(int line){
	int steps = introLadder[rung];
	
	// Steps
	displayNumber(steps, 3, LCD_POSITION_SCR, line, LCD_PADDING_ZERO);
	lcd.setCursor(LCD_POSITION_SCR + 3, line);
	lcd.print(":");
	
	// Count
	displayNumber(steps / 75, 1, LCD_POSITION_SCR + 4, line, LCD_PADDING_ZERO);
	lcd.setCursor(LCD_POSITION_SCR + 5, line);
	lcd.print(":");
	
	// Remainder
	displayNumber(steps % 75, 2, LCD_POSITION_SCR + 6, line, LCD_PADDING_ZERO);
}

void displayCurrentSCR(int line){
	// Steps
	displayNumber(currentSteps, 3, LCD_POSITION_SCR, line, LCD_PADDING_ZERO);
	lcd.setCursor(LCD_POSITION_SCR + 3, line);
	lcd.print(":");
	
	// Count
	displayNumber(currentSteps / 75, 1, LCD_POSITION_SCR + 4, line, LCD_PADDING_ZERO);
	lcd.setCursor(LCD_POSITION_SCR + 5, line);
	lcd.print(":");
	
	// Remainder
	displayNumber(currentSteps % 75, 2, LCD_POSITION_SCR + 6, line, LCD_PADDING_ZERO);
}

void displayJumps()
{
	displayNumber(currentJumps, LCD_POSITION_RIGHT - LCD_POSITION_SCR + 1, LCD_POSITION_SCR, LCD_LINE_ONE, LCD_PADDING_SPACE);
}

void updateDisplay(){
	if(displayState != programState){
		lcd.clear();
		lcd.setCursor(0,0);
		
		switch(programState){
			case STATE_SETUP:
				lcd.print("Select rung:");
				displayRung();
				displaySelectedSCR(LCD_LINE_ONE);
				break;
			case STATE_READY:
				lcd.print("S:C:R -");
				displaySelectedSCR(LCD_LINE_ZERO);
				lcd.setCursor(0, LCD_LINE_ONE);
				lcd.print("Step on to begin");
				break;
			case STATE_RUNNING:
				lcd.print("S:C:R -");
				displaySelectedSCR(LCD_LINE_ZERO);
				lcd.setCursor(0, LCD_LINE_ONE);
				lcd.print("Current:");
				displayCurrentSCR(LCD_LINE_ONE);
				break;
			case STATE_JUMPING:
				lcd.print("S:C:R -");
				displaySelectedSCR(LCD_LINE_ZERO);
				lcd.setCursor(0, LCD_LINE_ONE);
				lcd.print("JUMP!");
				displayJumps();
				break;
			case STATE_COMPLETE:
				lcd.print("DONE!");
				displaySelectedSCR(LCD_LINE_ZERO);
				lcd.setCursor(0, LCD_LINE_ONE);
				lcd.print("Current:");
				displayCurrentSCR(LCD_LINE_ONE);
				break;
			}
			
		displayState = programState;
	}
}

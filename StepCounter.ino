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
#define STATE_COMPLETE 0x4

#define LCD_POSITION_RUNG 14
#define LCD_POSITION_SCR 8

int programState;
int displayState;
int rung;
uint8_t buttonState;

int currentSteps;
int switchState;

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
	currentSteps = -1;
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
			if(currentSteps == 0){
				programState = STATE_RUNNING;
			}
			break;
		case STATE_RUNNING:
			processSwitch();
			if(currentSteps == introLadder[rung]){
				programState = STATE_COMPLETE;
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
			currentSteps++;
			
			if(programState == STATE_RUNNING || programState == STATE_COMPLETE){
				displayCurrentSCR(1);
			}
		}
	}
	
}

// void checkSwitchState()
// {
	// newSwitchState = digitalRead(35);
	// delay(10);
	// state2 = digitalRead(35);

	// if(newSwitchState == state2 && switchState != newSwitchState)
	// {
		// switchState = newSwitchState;
		//// stateChanges++;
		
		// lcd.clear();
		// lcd.setCursor(0,0);
		
		// if(switchState == HIGH)
		// {
			// lcd.print("HIGH");
		// }
		// else
		// {
			// lcd.print("LOW");
		// } 
		
		// lcd.setCursor(0,1);
		//// lcd.print(stateChanges);
	// }  
// }

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
		displaySelectedSCR(1);
	}
}

void displayNumber(int num, int width, int pos, int line){
	lcd.setCursor(pos, line);
	
	Serial.print("num: "); Serial.print(num); Serial.print(", width: "); Serial.print(width); Serial.print(", line: "); Serial.println(line);

	while(width > 0 && num < pow(10, --width)){
		lcd.print(0, 10);
		lcd.setCursor(++pos, line);
	}

	lcd.print(num, 10);
}

void displayRung(){
	displayNumber(rung, 2, LCD_POSITION_RUNG, 0);
}

void displaySelectedSCR(int line){
	int steps = introLadder[rung];
	
	// Steps
	displayNumber(steps, 3, LCD_POSITION_SCR, line);
	lcd.setCursor(LCD_POSITION_SCR + 3, line);
	lcd.print(":");
	
	// Count
	displayNumber(steps / 75, 1, LCD_POSITION_SCR + 4, line);
	lcd.setCursor(LCD_POSITION_SCR + 5, line);
	lcd.print(":");
	
	// Remainder
	displayNumber(steps % 75, 2, LCD_POSITION_SCR + 6, line);
}

void displayCurrentSCR(int line){
	// Steps
	displayNumber(currentSteps, 3, LCD_POSITION_SCR, line);
	lcd.setCursor(LCD_POSITION_SCR + 3, line);
	lcd.print(":");
	
	// Count
	displayNumber(currentSteps / 75, 1, LCD_POSITION_SCR + 4, line);
	lcd.setCursor(LCD_POSITION_SCR + 5, line);
	lcd.print(":");
	
	// Remainder
	displayNumber(currentSteps % 75, 2, LCD_POSITION_SCR + 6, line);
}

void updateDisplay(){
	if(displayState != programState){
		lcd.clear();
		lcd.setCursor(0,0);
		
		switch(programState){
			case STATE_SETUP:
				lcd.print("Select rung:");
				displayRung();
				displaySelectedSCR(1);
				break;
			case STATE_READY:
				lcd.print("S:C:R -");
				displaySelectedSCR(0);
				lcd.setCursor(0, 1);
				lcd.print("Step on to begin");
				break;
			case STATE_RUNNING:
				lcd.print("S:C:R -");
				displaySelectedSCR(0);
				lcd.setCursor(0, 1);
				lcd.print("Current:");
				displayCurrentSCR(1);
				break;
			case STATE_COMPLETE:
				lcd.print("DONE!");
				displaySelectedSCR(0);
				lcd.setCursor(0, 1);
				lcd.print("Current:");
				displayCurrentSCR(1);
				break;
			}
			
		displayState = programState;
	}
}

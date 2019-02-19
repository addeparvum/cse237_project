#include "assignment1.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <stdint.h>

void init_shared_variable(SharedVariable* sv) {
	// You can initialize the shared variable if needed.
	sv->bProgramExit = 0;
	sv->smallMic = 0;
	sv->bigMic = 0;
	sv->touchSensor = 0;
	sv->paused = 0;
}

void init_sensors(SharedVariable* sv) {
	pinMode(PIN_DIP_RED,OUTPUT);
	pinMode(PIN_DIP_GRN,OUTPUT);
	pinMode(PIN_DIP_BLU,OUTPUT);
	pinMode(PIN_BUTTON,INPUT);
	pinMode(PIN_BIG,INPUT);
	pinMode(PIN_SMALL,INPUT);
	pinMode(PIN_TOUCH,INPUT);
	pinMode(PIN_SMD_RED,OUTPUT);
	pinMode(PIN_SMD_GRN,OUTPUT);
	pinMode(PIN_SMD_BLU,OUTPUT);
	softPwmCreate(PIN_SMD_RED, 0, 0xFF);
	softPwmCreate(PIN_SMD_GRN, 0, 0xFF);
    softPwmCreate(PIN_SMD_BLU, 0, 0xFF);	
	pinMode(PIN_ALED,OUTPUT);
	pinMode(PIN_BUZZER,OUTPUT);

}

void body_button(SharedVariable* sv) {
	int val = 0;
		val = digitalRead(PIN_BUTTON);
		if(val == LOW){
			if(sv->paused == 0){
				sv->paused = 1;
			} else {
				sv->paused = 0;
			}
		}
	
	//sv->bProgramExit = 1;
}

void body_threecolor(SharedVariable* sv) {
		if(sv->paused == 1){
			digitalWrite(PIN_DIP_RED,LOW);
			digitalWrite(PIN_DIP_BLU,LOW);
			digitalWrite(PIN_DIP_GRN,LOW);
		}
		
		else if((sv->smallMic) == 0){
			digitalWrite(PIN_DIP_RED,LOW);
			digitalWrite(PIN_DIP_BLU,HIGH);
			digitalWrite(PIN_DIP_GRN,LOW);
		} else if((sv->smallMic) == 1){
			digitalWrite(PIN_DIP_RED,HIGH);
			digitalWrite(PIN_DIP_BLU,LOW);
			digitalWrite(PIN_DIP_GRN,LOW);
		}
}

void body_big(SharedVariable* sv) {
	int val;
		val = digitalRead(PIN_BIG);
		if(val == HIGH){
			sv->bigMic = 1;
		} else {
			sv->bigMic = 0;
		}
}

void body_small(SharedVariable* sv) {
	int val;
		val = digitalRead(PIN_SMALL);
		if(val == HIGH){
			sv->smallMic = 1;
		} else {
			sv->smallMic = 0;
		}
}

void body_touch(SharedVariable* sv) {
	int val;
		val = digitalRead(PIN_TOUCH);
		if(val == HIGH){
			sv->touchSensor = 1;
		} else {
			sv->touchSensor = 0;
		}
}

void body_rgbcolor(SharedVariable* sv) {
		if(sv->paused == 1){
			softPwmWrite(PIN_SMD_RED,0x00);
			softPwmWrite(PIN_SMD_GRN,0x00);
			softPwmWrite(PIN_SMD_BLU,0x00);			
		}
		else if((sv->smallMic) == 0 &&(sv->touchSensor) == 0){
			softPwmWrite(PIN_SMD_RED,0xff);
			softPwmWrite(PIN_SMD_GRN,0x00);
			softPwmWrite(PIN_SMD_BLU,0x00);			
		} else if((sv->smallMic) == 1 &&(sv->touchSensor) == 0){
			softPwmWrite(PIN_SMD_RED,0xee);
			softPwmWrite(PIN_SMD_GRN,0x00);
			softPwmWrite(PIN_SMD_BLU,0xc8);			
		
		} else if((sv->smallMic) == 0 &&(sv->touchSensor) == 1){
			softPwmWrite(PIN_SMD_RED,0x80);
			softPwmWrite(PIN_SMD_GRN,0xff);
			softPwmWrite(PIN_SMD_BLU,0x00);			
		
		} else if((sv->smallMic) == 1 &&(sv->touchSensor) == 1){
			softPwmWrite(PIN_SMD_RED,0x00);
			softPwmWrite(PIN_SMD_GRN,0xff);
			softPwmWrite(PIN_SMD_BLU,0xff);			
		}
	
	
}

void body_aled(SharedVariable* sv) {
		if(sv->paused == 1){
			digitalWrite(PIN_ALED,LOW);
		} else {
			digitalWrite(PIN_ALED,HIGH);
		}
	
}

void body_buzzer(SharedVariable* sv) {
	int count = 0;
		if((sv->bigMic) == 1){
			while(count++ < 250){
				digitalWrite(PIN_BUZZER,HIGH);
				delay(1);
				digitalWrite(PIN_BUZZER,LOW);
				delay(1);
			}
			count = 0;
		} else {
			digitalWrite(PIN_BUZZER,LOW);
		}
}


#include "assignment1.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <stdint.h>

void init_shared_variable(SharedVariable* sv) {
	// You can initialize the shared variable if needed.
	sv->bProgramExit = 0;
}

void init_sensors(SharedVariable* sv) {
}

void body_button(SharedVariable* sv) {
	sv->bProgramExit = 1;
}

void body_threecolor(SharedVariable* sv) {
}

void body_big(SharedVariable* sv) {
}

void body_small(SharedVariable* sv) {
}

void body_touch(SharedVariable* sv) {
}

void body_rgbcolor(SharedVariable* sv) {
}

void body_aled(SharedVariable* sv) {
}

void body_buzzer(SharedVariable* sv) {
}
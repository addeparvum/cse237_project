// Important! DO NOT MODIFY this file.
// You will not submit this file.
// This file is provided for your understanding of the program procedure.

// Skeleton code of CSE237A, Sensor interaction
// For more details, please see the instructions in the class website


#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "assignment1.h"

#define thread_decl(NAME) \
void* thread_##NAME(void* param) { \
	SharedVariable* pV = (SharedVariable*) param; \
	body_##NAME(pV); \
	return NULL; }

thread_decl(button)
thread_decl(threecolor)
thread_decl(big)
thread_decl(small)
thread_decl(touch)
thread_decl(rgbcolor)
thread_decl(aled)
thread_decl(buzzer)

#define thread_create(NAME) pthread_create(&t_##NAME, NULL, thread_##NAME, &v);
#define thread_join(NAME) pthread_join(t_##NAME, NULL);

int main(int argc, char* argv[]) {
	// Initialize
	SharedVariable v;

	if (wiringPiSetup() == -1) {
		printf("Failed to setup wiringPi.");
		return 1; 
	}

	init_shared_variable(&v);
	init_sensors(&v);

	pthread_t t_button,
			  t_threecolor,
			  t_big,
			  t_small,
			  t_touch,
			  t_rgbcolor,
			  t_aled,
			  t_buzzer;

	while (v.bProgramExit != 1) {
		// Create sensing threads
		thread_create(button);
		thread_create(threecolor);
		thread_create(big);
		thread_create(small);
		thread_create(touch);
		thread_create(rgbcolor);
		thread_create(aled);
		thread_create(buzzer);

		// Wait until all threads finish
		thread_join(button);
		thread_join(threecolor);
		thread_join(big);
		thread_join(small);
		thread_join(touch);
		thread_join(rgbcolor);
		thread_join(aled);
		thread_join(buzzer);

		delay(1);
	}

	printf("Program finished.\n");

	return 0;
}

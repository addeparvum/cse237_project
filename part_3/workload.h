// Important! DO NOT MODIFY this file.
// You will not submit this file.
// You need to schedule specified workloads in this file.

#ifndef _WORKLOAD_HERADER_
#define _WORKLOAD_HERADER_

// Header file for virtual workloads
// DO NOT modify anything in this file if you couldn't guarantee.

#define NUM_TASKS 8

// Task IDs
enum {
	BUTTON = 0,
	THREECOLOR,
	BIG,
	SMALL,
	TOUCH,
	RGBCOLOR,
	ALED,
	BUZZER
};

#define thread_decl(NAME) \
extern int bExit_##NAME; \
void* thread_##NAME(void* param);

// Virtual workload + Body of sensing
thread_decl(button);
thread_decl(threecolor);
thread_decl(big);
thread_decl(small);
thread_decl(touch);
thread_decl(rgbcolor);
thread_decl(aled);
thread_decl(buzzer);

// Virtual workload
void init_workload();
void finish_workload();

void workload_button();
void workload_threecolor();
void workload_big();
void workload_small();
void workload_touch();
void workload_rgbcolor();
void workload_aled();
void workload_buzzer();

// Deadlines for 8 tasks (workload)
// Also see deadlines.c
extern long long workloadDeadlines[];

#endif

#include "assignment1.h"
#include "assignment2.h"
#include "workload.h"
#include "scheduler.h"
#include "governor.h"

// Note: Deadline of each workload is defined in the "workloadDeadlines" variable.
// i.e., You can access the dealine of the BUTTON thread using workloadDeadlines[BUTTON]
// See also deadlines.c and workload.h

// Assignment: You need to implement the following two functions.

// learn_workloads(SharedVariable* v):
// This function is called at the start part of the program before actual scheduling
// - Parameters
// sv: The variable which is shared for every function over all threads
void learn_workloads(SharedVariable* sv) {
	// TODO: Fill the body
	// This function is executed before the scheduling simulation.
	// You need to calculate the execution time of each thread here.

	// Thread functions for workloads: 
	// thread_button, thread_threecolor, thread_big, thread_small,
	// thread_touch, thread_rgbcolor, thread_aled, thread_buzzer

	// Tip 1. You can call each workload function here like:
	// thread_button();

	// Tip 2. You can get the current time here like:
	// long long curTime = get_current_time_us();

	for(int i = 0; i < 8; i++){
		(sv->nextDeadline)[i] = workloadDeadlines[i];
		(sv->aliveDup)[i] = 1;
		sv->prevSelc = -1;
		sv->freq[i] = 1;
	}
    	
//	printDBG("%llu",workloadDeadlines[0]);
	double execRatios[8];
	double sumRatios = 0.00;
	long long curTime = get_current_time_us();
	thread_button(sv); 
	execRatios[0] = (double)(get_current_time_us() - curTime)/workloadDeadlines[BUTTON];

	curTime = get_current_time_us();
	thread_threecolor(sv);
	execRatios[1] = (double)(get_current_time_us() - curTime)/workloadDeadlines[THREECOLOR];

	curTime = get_current_time_us();
	thread_big(sv);
	execRatios[2] = (double)(get_current_time_us() - curTime)/workloadDeadlines[BIG];

	curTime = get_current_time_us();
	thread_small(sv);
	execRatios[3] = (double)(get_current_time_us() - curTime)/workloadDeadlines[SMALL];

	curTime = get_current_time_us();
	thread_touch(sv);
	execRatios[4] = (double)(get_current_time_us() - curTime)/workloadDeadlines[TOUCH];

	curTime = get_current_time_us();
	thread_rgbcolor(sv);
	execRatios[5] = (double)(get_current_time_us() - curTime)/workloadDeadlines[RGBCOLOR];

	curTime = get_current_time_us();
	thread_aled(sv);
	execRatios[6] = (double)(get_current_time_us() - curTime)/workloadDeadlines[ALED];

	curTime = get_current_time_us();
	thread_buzzer(sv);
	execRatios[7] = (double)(get_current_time_us() - curTime)/workloadDeadlines[BUZZER];
	

	int triedDouble[8];
	for(int i = 0; i < 8; i++){
		sumRatios += execRatios[i];
		triedDouble[i] = 0;
	}
	for(int i = 0; i < 8; i++){
		double largest = -1.2;  
		int largestIndex = -1;
		for(int i = 0; i < 8; i++){
			if(triedDouble[i] == 0 && execRatios[i] > largest){
				largest = execRatios[i];
				largestIndex = i;
			}
		}
		if((sumRatios + execRatios[largestIndex]) < 0.90){
			sumRatios += execRatios[largestIndex];
			sv->freq[largestIndex] = 0;
		}
		triedDouble[largestIndex] = 1;
	}
	
	return;

}


// select_task(SharedVariable* sv, const int* aliveTasks):
// This function is called while runnning the actual scheduler
// - Parameters
// sv: The variable which is shared for every function over all threads
// aliveTasks: an array where each element indicates whether the corresponed task is alive(1) or not(0).
// idleTime: a time duration in microsecond. You can know how much time was waiting without any workload
//           (i.e., it's larger than 0 only when all threads are finished and not reache the next preiod.)
// - Return value
// TaskSelection structure which indicates the scheduled task and the CPU frequency
TaskSelection select_task(SharedVariable* sv, const int* aliveTasks, long long idleTime) {
	// TODO: Fill the body
	// This function is executed inside of the scheduling simulation.
    // You need to implement an energy-efficient EDF (Earliest Deadline First) scheduler.

	// Tip 1. You may get the current time elapsed in the scheduler here like:
	// long long curTime = get_scheduler_elapsed_time_us();

	// Also, do not make any interruptable / IO tasks in this function.
	// You can use printfDBG instead of printf.

	// Sample scheduler: Round robin
	// It selects a next thread using aliveTasks.
/*	static int prev_selection = -1;

	int i = prev_selection + 1;
	while(1) {
		if (i == NUM_TASKS)
			i = 0;

		if (aliveTasks[i] == 1) {
			prev_selection = i;
			break;
		}
		++i;
	}

	// The retun value can be specified like this:
	TaskSelection sel;
	sel.task = prev_selection; // The thread ID which will be scheduled. i.e., 0(BUTTON) ~ 7(BUZZER)
	sel.freq = 1; // Request the maximum frequency (if you want the minimum frequency, use 0 instead.)
*/
	TaskSelection sel;

	if(sv->prevSelc == -1){
	 	int eDLIndex = 0;
		int earliestDeadline = sv->nextDeadline[0];
		for(int i = 1; i < 8; i++){
			if(sv->nextDeadline[i] < earliestDeadline && aliveTasks[i] == 1){
				eDLIndex = i;
			}
		}
		sel.task = eDLIndex;
		sel.freq = sv->freq[eDLIndex];
		sv->prevSelc = eDLIndex;
		return sel;

	} else {
		if(sv->aliveDup[sv->prevSelc] == 1 && aliveTasks[sv->prevSelc] == 0){
			sv->nextDeadline[sv->prevSelc] += workloadDeadlines[sv->prevSelc];
			int ind;
			int eDLIndex;
			long long earliestDeadline;
			for(ind = 0; ind < 8; ind++){
				if(aliveTasks[ind] == 1){
					eDLIndex = ind;
					earliestDeadline = sv->nextDeadline[ind];
					break;		
				}
			}		

			for(ind; ind < 8; ind++){
				if(sv->nextDeadline[ind] < earliestDeadline && aliveTasks[ind] == 1){
					earliestDeadline = sv->nextDeadline[ind];
					eDLIndex = ind;
				}	
			}
			sel.task = eDLIndex;
			sel.freq = sv->freq[eDLIndex];
			sv->prevSelc = eDLIndex;
			return sel;
			
		} else {
			for(int i = 0; i < 8; i++){
				if(sv->aliveDup[i] == 0 && aliveTasks[i] == 1){
					if(sv->nextDeadline[i] < sv->nextDeadline[sv->prevSelc]){
						sel.task = i;
						sv ->prevSelc = i;
						sel.freq = sv->freq[i];
						return sel;
					}
				}
			}	
			
		} 
	
	}	

	sel.task = sv->prevSelc;
	sel.freq = sv->freq[sv->prevSelc];
    return sel;
}

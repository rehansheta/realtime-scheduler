/********************************************************************************************************
* Main.cpp
*
* simulation for PO-FPS algorithm on a single-processor system
*
* created by Rehana Begam on February, 2013
* 
* Declared as COMPLETE and FINAL on February 25, 2014 
*
********************************************************************************************************


update history over 1 year !!!!!
********************************************************************************************************
* Updates on February 24, 2014:
*
* 1. corrected the arriving event problem for ALAP tasks
* 2. added the calculation for overload and invocation
* 3. removed the cout dependencies
*
* TODO :
*
* 1. add the PERIOD TRANSFORMATION scheme
* 2. check the AREA_UNDER_CURVE (time integration) again (optional)
********************************************************************************************************
* Updates on February 20, 2014:
*
* 1. ported on linux
* 2. system time problem is resolved
********************************************************************************************************
* Updates on November 10, 2013:
*
* 1. restructure the code
********************************************************************************************************
* Updates on November 1, 2013:
*
* 1. during slack management, preemption is considered correctly.
* 2. Slack is treated as a task and the management is corrected.
* 3. push forwarding of slack is done.
* 4. DUMMY TASK is implemented.
* 
* TODO :
*
* 1. randomization of dynamic load
* 2. restructure the code
********************************************************************************************************
* Updates on October 22, 2013:
*
* 1. during slack management, preemption is considered correctly.
* 2. Slack is treated as a task and the management is corrected.
* 
* TODO :
*
* 1. randomization of dynamic load
* 2. restructure the code
********************************************************************************************************
* Updates on September 25, 2013:
*
* 1. PV value equation for asap task is modified while alap is unchanged.
* 2. during slack management, preemption is considered correctly.
* 
* TODO :
*
* 1. randomization of dynamic load
********************************************************************************************************
* Updates on September 1, 2013:
*
* 1. Dynamic adjustment for PV value is done in the upper queue
* 2. Paper link: 
* 3. PV-value calculation is adjusted with 'exet'
* 4. PQV class is used instead of priority_queue for the vector extraction purpose
********************************************************************************************************
* Updates on July 15, 2013:
*
* 1. PV value is corrected
* 2. two variations for PV values are formulated and the previous one is selected
********************************************************************************************************
* Updates on June 9, 2013:
*
* 1. AREA_UNDER_CURVE (time integration) is calculated as a performance metric.
********************************************************************************************************
* Updates on June 4, 2013:
*
* 1. Fixed number of task = 20
* 2. from task_count = 10 to task_count = 100
* 3. min_period = 10, max_period = 100
* 4. period calculation corrected.
* 5. from util = 0.5 to util = .65
********************************************************************************************************
* Finished:
*
* 1. Task creation
* 2. LCM
* 3. Priority Queue
* 4. Preemption logic
* 5. Timeline/Event
* 6. Promotion time
* 7. Equality of period in Comparator
* 8. Separate class for PO-RMS
* 9. Parformance measurement
* 10. Update the periodic re-appearence: use another priority queue
* 11. Running in loop
* 12. Generate task sets
* 13. Check the stat for PV_value
*********************************************************************************************************
* TODO: 
*
* 1. 
* 2. Do some more Parformance measurement
********************************************************************************************************
*	QUERY: 
*
* 1. Is the promotion time OK ??! ( done with Tj => period, prioritizing task on period..)
* 2. Is the PV_RMS calculation OK ??! (no preference_level is considered...)
********************************************************************************************************/


#include <iostream>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <sstream>
#ifdef PORMS_WINDOWS
#include <windows.h>
#endif

#include "QComparators.h"
#include "Constants.h"
#include "TaskSetGenerator.h"
#include "Tasks.h"
#include "RTScheduling.h"
#include "Utility.h"


using namespace std;

int ASAP_TASK_COUNT;
int ALAP_TASK_COUNT;
double ASAP_TASK_UTILIZATION;
double ALAP_TASK_UTILIZATION;

double TOTAL_UTILIZATION;

int TOTAL_TASKS;

double MIN_U;
double MAX_U;

ofstream DTtrace;
ofstream cout_trace;
ofstream tracePV_noDummy;	// for tracing PV--no dummy
ofstream tracePV_Dummy;		// for tracing PV--with dummy
ofstream tracePV_noSlack;	// for tracing PV--no slack
ofstream tracePV_Slack;		// for tracing PV--with slack

int main ()
{
	int lcm;
	int withSlack = 1;
	double PORMS_start_timer;
	double PORMS_finish_timer;
	double PORMS_start_timer_dummy;
	double PORMS_finish_timer_dummy;
	double PORMS_start_timer_slack;
	double PORMS_finish_timer_slack;
	double RMS_start_timer;
	double RMS_finish_timer;
	int random_seed;

	double min_period;

	ofstream genPV_noDummy;		// for generating PV--no dummy
	ofstream genPV_Dummy;		// for generating PV--with dummy
	ofstream genPV_noSlack;		// for generating PV--no slack
	ofstream genPV_Slack;		// for generating PV--with slack

	genPV_noDummy.open("PV_T10-100_PORMSDummySlack/genPV_noDT.out");
	tracePV_noDummy.open("PV_T10-100_PORMSDummySlack/tracePV_noDT.out");

	genPV_Dummy.open("PV_T10-100_PORMSDummySlack/genPV_DT.out");
	tracePV_Dummy.open("PV_T10-100_PORMSDummySlack/tracePV_DT.out");

	genPV_noSlack.open("PV_T10-100_PORMSDummySlack/genPV_noDT_noSlack.out");
	tracePV_noSlack.open("PV_T10-100_PORMSDummySlack/tracePV_noDT_noSlack.out");

	genPV_Slack.open("PV_T10-100_PORMSDummySlack/genPV_noDT_Slack.out");
	tracePV_Slack.open("PV_T10-100_PORMSDummySlack/tracePV_noDT_Slack.out");

	DTtrace.open("PV_T10-100_PORMSDummySlack/dummy_DT.trace");
	cout_trace.open("PV_T10-100_PORMSDummySlack/cout_noSlack.txt");

	//writeHeaders(genPV_noDummy, genPV_Dummy, genPV_noSlack, genPV_Slack, tracePV_noDummy, tracePV_Dummy, tracePV_noSlack, tracePV_Slack);

	for (int tot_task = 10; tot_task <= 100; tot_task = tot_task + 10) 
	{

	TOTAL_TASKS = tot_task;	
	genPV_noDummy << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;
	//tracePV_noDummy << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;

	genPV_Dummy << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;
	//tracePV_Dummy << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;

	genPV_noSlack << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;
	//tracePV_noSlack << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;

	genPV_Slack << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;
	//tracePV_Slack << "\nTOTAL_TASKS: " << TOTAL_TASKS << endl;

	/*int dyn_load = 100;
	double task_dyn_load = dyn_load/100.0;
	genPV_noDummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
	tracePV_noDummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;

	genPV_Dummy << "TASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
	tracePV_Dummy << "TASK_DYNAMIC_LOAD: " << task_dyn_load << endl;*/

	//for (double util = 0.1; util <= 0.7; util = util + 0.05) 
	//{
		TOTAL_UTILIZATION =	0.5;	//util;
		genPV_noDummy << "TOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;
		//tracePV_noDummy << "TOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;

		genPV_Dummy << "\nTOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;
		//tracePV_Dummy << "\nTOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;

		genPV_noSlack << "TOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;
		//tracePV_noSlack << "TOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;

		genPV_Slack << "\nTOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;
		//tracePV_Slack << "\nTOTAL_UTILIZATION: " << TOTAL_UTILIZATION << endl;

		//for (int dyn_load = 20; dyn_load <= 100; dyn_load += 10)
		//{
		int dyn_load = 100;
		double task_dyn_load = dyn_load/100.0;
		genPV_noDummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
		//tracePV_noDummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;

		genPV_Dummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
		//tracePV_Dummy << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;

		genPV_noSlack << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
		//tracePV_noSlack << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;

		genPV_Slack << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;
		//tracePV_Slack << "\nTASK_DYNAMIC_LOAD: " << task_dyn_load << endl;

		for (int task_count = TOTAL_TASKS * 0.2; task_count < TOTAL_TASKS; task_count = task_count + (0.3 * TOTAL_TASKS))
		{
			// statistics
			double tot_area = 0.0;
			double tot_area_rms = 0.0;
			double avg_area_ratio = 0.0;
			double tot_area_dummy = 0.0;
			double avg_area_ratio_dummy = 0.0;
			double tot_area_slack = 0.0;
			double avg_area_ratio_slack = 0.0;
			double area_ratio = 0.0;
			double area_ratio_rms = 0.0;
			double area_ratio_dummy = 0.0;
			double area_ratio_slack = 0.0;

			double tot_time_PORMS = 0;
			double tot_time_PORMS_dummy = 0;
			double tot_time_PORMS_slack = 0;
			double tot_time_RMS = 0;

			int PORMS_schedule_point_num = 0;
			int RMS_schedule_point_num = 0;
			int PORMS_schedule_point_num_dummy = 0;
			int PORMS_schedule_point_num_slack = 0;

			int tot_PORMS_schedule_point_num = 0;
			int tot_RMS_schedule_point_num = 0;
			int tot_PORMS_schedule_point_num_dummy = 0;
			int tot_PORMS_schedule_point_num_slack = 0;

			double tot_PV_PORMS = 0;
			double tot_PV_PORMS_ASAP = 0;
			double tot_PV_PORMS_ALAP = 0;
			double tot_PV_PORMS_dummy = 0;
			double tot_PV_PORMS_ASAP_dummy = 0;
			double tot_PV_PORMS_ALAP_dummy = 0;
			double tot_PV_PORMS_slack = 0;
			double tot_PV_PORMS_ASAP_slack = 0;
			double tot_PV_PORMS_ALAP_slack = 0;
			
			double tot_PV_RMS = 0;
			double tot_PV_RMS_ASAP = 0;
			double tot_PV_RMS_ALAP = 0;

			double tot_prom_time_dummy = 0;
			double tot_prom_time_slack = 0;

			double avg_schedule_time_PORMS = 0;
			double avg_schedule_time_PORMS_dummy = 0;
			double avg_schedule_time_PORMS_slack = 0;
			double avg_schedule_time_RMS = 0;

			//int task_count = 2;
			ASAP_TASK_COUNT = task_count;
			ALAP_TASK_COUNT = TOTAL_TASKS - task_count;

			ASAP_TASK_UTILIZATION = TOTAL_UTILIZATION * (double)ASAP_TASK_COUNT / (double)TOTAL_TASKS;
			ALAP_TASK_UTILIZATION = TOTAL_UTILIZATION * (double)ALAP_TASK_COUNT / (double)TOTAL_TASKS;

			MIN_U = ASAP_TASK_UTILIZATION / (double)ASAP_TASK_COUNT - 1;
			MAX_U = 0.5;

			// start the simulation...
			for (int i = 0; i < TASK_SETS_COUNT; i++) 
			{
				CTaskSetGenerator* generateTask = new CTaskSetGenerator();
				CRTScheduling* RTSchedule = new CRTScheduling();
				CTasks* dummy_task = new CTasks();

				random_seed = GetTimeMs64();
				srand32(random_seed);	//for period generation
				srand(random_seed);		//for utilization generation
				
				min_period = generateTask->GeneratePORMSTaskSet(task_dyn_load);
				generateTask->generateRMSTaskSet();
				lcm = generateTask->getTaskSetLCM();	// get the lcm

				RTSchedule->LCM_for_area = lcm;

				RTSchedule->PV_PORMS[0] = 0.0;
				for(int pv = 1; pv < TOTAL_TASKS + 1; pv++)
				{
					RTSchedule->PV_PORMS[pv] = 0.0;
					RTSchedule->PV_RMS[pv - 1] = 0.0;
				}

				PQV <CTasks, CPeriodComparator> highReadyQueue;				// priority queues for asap+alap tasks:: no_dummy			
				PQV <CTasks, CPromotionComparator> lowReadyQueue;

				PQV <CTasks, CPeriodComparator> highReadyQueue_Dummy;	    // priority queues for asap+alap tasks:: with_dummy
				PQV <CTasks, CArrivalComparator> arrivalQueue_Dummy;		// priority queue for arriving porms tasks:: with_dummy
				PQV <CTasks, CPromotionComparator> lowReadyQueue_Dummy;

				PQV <CTasks, CArrivalComparator> arrivalQueue;				// priority queue for arriving porms tasks (for schemes: porms n slack)

				PQV <CTasks, CArrivalComparator> arrivalQueue_RMS;			// priority queue for arriving rms tasks
				PQV <CTasks, CPeriodComparator> singleReadyQueue;			// priority queues for rms tasks

				for (int j = 1; j < ASAP_TASK_COUNT + 1; j++)
				{
					//highReadyQueue.push(generateTask->asap[j]);
					arrivalQueue.push(generateTask->asap[j]);
				}
				for (int j = 1; j < ALAP_TASK_COUNT + 1; j++)
				{
					//lowReadyQueue.push(generateTask->alap[j]);
					arrivalQueue.push(generateTask->alap[j]);
				}
				for (int j = 0; j < TOTAL_TASKS; j++)
				{
					//singleReadyQueue.push(generateTask->tasks[j]);
					arrivalQueue_RMS.push(generateTask->tasks[j]);
				}

#ifdef use_dummy
				if (TOTAL_UTILIZATION < MAX_RMS_LOAD)
				{
					generateTask->GeneratePORMSTaskSet_dummy(task_dyn_load, min_period);
					//create the dummy task
					for (int j = 0; j < ASAP_TASK_COUNT + 1; j++)
					{
						//highReadyQueue_Dummy.push(generateTask->asap[j]);
						arrivalQueue_Dummy.push(generateTask->asap[j]);
					}
					for (int j = 1; j < ALAP_TASK_COUNT + 1; j++)
					{
						//lowReadyQueue_Dummy.push(generateTask->alap_dummy[j]);
						arrivalQueue_Dummy.push(generateTask->alap_dummy[j]);
					}
				}
#endif
				//generateTask->printAllTasks(i);

				//SIMULATTION#1 PORMS--noDUMMY, noSLACK
				RTSchedule->PORMS_schedule_point_num = 0.0;
				RTSchedule->porms_taskset_area = 0.0;
				RTSchedule->porms_taskset_area_point = 0.0;

				PORMS_start_timer = GetTimeMs64();
				int schedulable = RTSchedule->runPORMS(highReadyQueue, lowReadyQueue, arrivalQueue, lcm, generateTask, !withSlack);
				PORMS_finish_timer = GetTimeMs64();
				if (schedulable == -1) {
					DTtrace << "@@@@@@@@@ " << i << "th task for PORMS was not schedulable @@@@@@@@@\n\n" << endl;
					i--;
					delete[] generateTask->asap;
					delete[] generateTask->alap;
					delete[] generateTask->tasks;
					continue;
				}
				double one_time_PORMS = PORMS_finish_timer - PORMS_start_timer;
				avg_schedule_time_PORMS += one_time_PORMS / (double)RTSchedule->PORMS_schedule_point_num;
				tot_time_PORMS += one_time_PORMS;
				PORMS_schedule_point_num = RTSchedule->PORMS_schedule_point_num;
				tot_PORMS_schedule_point_num += PORMS_schedule_point_num;
				tot_area = RTSchedule->porms_taskset_area;
				
				//RMS
				RTSchedule->RMS_schedule_point_num = 0.0;
				RTSchedule->rms_taskset_area = 0.0;
				RTSchedule->rms_taskset_area_point = 0.0;

				RMS_start_timer = GetTimeMs64();
				schedulable = RTSchedule->runRMS(singleReadyQueue, arrivalQueue_RMS, lcm, generateTask);
				RMS_finish_timer = GetTimeMs64();
				if (schedulable == -1) {
					DTtrace << "@@@@@@@@@ " << i << "th task for RMS was not schedulable @@@@@@@@@\n\n" << endl;
					i--;
					tot_time_PORMS -= one_time_PORMS;
					avg_schedule_time_PORMS -= one_time_PORMS / (double)PORMS_schedule_point_num;
					tot_PORMS_schedule_point_num -= PORMS_schedule_point_num;
					tot_area = 0;
					delete[] generateTask->asap;
					delete[] generateTask->alap;
					delete[] generateTask->tasks;
					continue;
				}			
				double one_time_RMS = RMS_finish_timer - RMS_start_timer;
				avg_schedule_time_RMS += one_time_RMS / (double)RTSchedule->RMS_schedule_point_num;		
				tot_time_RMS += one_time_RMS;
				RMS_schedule_point_num = RTSchedule->RMS_schedule_point_num;
				tot_RMS_schedule_point_num += RMS_schedule_point_num;
				tot_area_rms = RTSchedule->rms_taskset_area;

				//CALCULATE RESULTS for PORMS--noDUMMY, noSlack
				double avg_PV_PORMS = 0;
				double avg_PV_PORMS_ASAP = 0;
				double avg_PV_PORMS_ALAP = 0;

				//generateTask->printAllTasks(i);

				for (int j = 1; j < TOTAL_TASKS + 1; j++)
				{
					RTSchedule->PV_PORMS[j] /= ((double)lcm/generateTask->task_periods[j - 1]);
					avg_PV_PORMS += RTSchedule->PV_PORMS[j];
					if (generateTask->tasks[j - 1].preference_level == 1)
					{
						avg_PV_PORMS_ASAP += RTSchedule->PV_PORMS[j];
					}
					else
					{
						avg_PV_PORMS_ALAP += RTSchedule->PV_PORMS[j];
					}
				}
				avg_PV_PORMS /= (double)(TOTAL_TASKS);
				avg_PV_PORMS_ASAP /= (double)(ASAP_TASK_COUNT);
				avg_PV_PORMS_ALAP /= (double)(ALAP_TASK_COUNT);
				tot_PV_PORMS += avg_PV_PORMS;
				tot_PV_PORMS_ASAP += avg_PV_PORMS_ASAP;
				tot_PV_PORMS_ALAP += avg_PV_PORMS_ALAP;

				double avg_PV_RMS = 0;
				double avg_PV_RMS_ASAP = 0;
				double avg_PV_RMS_ALAP = 0;

				for (int j = 0; j < TOTAL_TASKS; j++)
				{
					RTSchedule->PV_RMS[j] /= ((double)lcm/generateTask->task_periods[j]);
					avg_PV_RMS += RTSchedule->PV_RMS[j];
					if (generateTask->tasks[j].preference_level == 1)
					{
						avg_PV_RMS_ASAP += RTSchedule->PV_RMS[j];
					}
					else
					{
						avg_PV_RMS_ALAP += RTSchedule->PV_RMS[j];
					}
				}

				avg_PV_RMS /= (double)(TOTAL_TASKS);
				avg_PV_RMS_ASAP /= (double)(ASAP_TASK_COUNT);
				avg_PV_RMS_ALAP /= (double)(ALAP_TASK_COUNT);
				tot_PV_RMS += avg_PV_RMS;
				tot_PV_RMS_ASAP += avg_PV_RMS_ASAP;
				tot_PV_RMS_ALAP += avg_PV_RMS_ALAP;

				//SIMULATTION#2 PORMS--withDUMMY, noSlack
#ifdef use_dummy
				double one_time_PORMS_dummy = 0.0;
				if (TOTAL_UTILIZATION < MAX_RMS_LOAD)
				{
					RTSchedule->PV_PORMS[0] = 0.0;
					for(int pv = 1; pv < TOTAL_TASKS + 1; pv++)
					{
						RTSchedule->PV_PORMS[pv] = 0.0;
					}
					RTSchedule->PORMS_schedule_point_num = 0.0;
					RTSchedule->porms_taskset_area = 0.0;
					RTSchedule->porms_taskset_area_point = 0.0;

					PORMS_start_timer_dummy = GetTimeMs64();
					int schedulable = RTSchedule->runPORMS(highReadyQueue_Dummy, lowReadyQueue_Dummy, arrivalQueue_Dummy, lcm, generateTask, !withSlack);
					PORMS_finish_timer_dummy = GetTimeMs64();
					if (schedulable == -1) {
						DTtrace << "@@@@@@@@@ " << i << "th task for PORMS was not schedulable @@@@@@@@@\n\n" << endl;
						i--;
						tot_time_PORMS -= one_time_PORMS;
						tot_time_RMS -= one_time_RMS;
						avg_schedule_time_PORMS -= one_time_PORMS / (double)PORMS_schedule_point_num;
						avg_schedule_time_RMS -= one_time_RMS / (double)RMS_schedule_point_num;
						tot_PORMS_schedule_point_num -= PORMS_schedule_point_num;
						tot_RMS_schedule_point_num -= RMS_schedule_point_num;
						tot_area = 0;
						tot_area_rms = 0;
						tot_PV_PORMS -= avg_PV_PORMS;
						tot_PV_PORMS_ASAP -= avg_PV_PORMS_ASAP;
						tot_PV_PORMS_ALAP -= avg_PV_PORMS_ALAP;
						tot_PV_RMS -= avg_PV_RMS;
						tot_PV_RMS_ASAP -= avg_PV_RMS_ASAP;
						tot_PV_RMS_ALAP -= avg_PV_RMS_ALAP;
						delete[] generateTask->asap;
						delete[] generateTask->alap;
						delete[] generateTask->tasks;
						continue;
					}

					//generateTask->printAllTasks(i);

					one_time_PORMS_dummy = PORMS_finish_timer_dummy - PORMS_start_timer_dummy;
					avg_schedule_time_PORMS_dummy += one_time_PORMS_dummy / (double)RTSchedule->PORMS_schedule_point_num;
					tot_time_PORMS_dummy += one_time_PORMS_dummy;
					PORMS_schedule_point_num_dummy = RTSchedule->PORMS_schedule_point_num;
					tot_PORMS_schedule_point_num_dummy += PORMS_schedule_point_num_dummy;
					tot_area_dummy = RTSchedule->porms_taskset_area;

					//CALCULATE RESULTS for PORMS--withDUMMY, noSlack
					double avg_PV_PORMS_dummy = 0;
					double avg_PV_PORMS_ASAP_dummy = 0;
					double avg_PV_PORMS_ALAP_dummy = 0;
					double avg_prom_time_dummy = 0;
					double nor_Prom_time = 0;

					for (int j = 1; j < TOTAL_TASKS + 1; j++)
					{
						RTSchedule->PV_PORMS[j] /= ((double)lcm/generateTask->task_periods[j - 1]);
						avg_PV_PORMS_dummy += RTSchedule->PV_PORMS[j];
						if (generateTask->tasks[j - 1].preference_level == 1)
						{
							avg_PV_PORMS_ASAP_dummy += RTSchedule->PV_PORMS[j];
						}
						else
						{
							avg_PV_PORMS_ALAP_dummy += RTSchedule->PV_PORMS[j];
						}
					}
					avg_PV_PORMS_dummy /= (double)(TOTAL_TASKS);
					avg_PV_PORMS_ASAP_dummy /= (double)(ASAP_TASK_COUNT);
					avg_PV_PORMS_ALAP_dummy /= (double)(ALAP_TASK_COUNT);
					tot_PV_PORMS_dummy += avg_PV_PORMS_dummy;
					tot_PV_PORMS_ASAP_dummy += avg_PV_PORMS_ASAP_dummy;
					tot_PV_PORMS_ALAP_dummy += avg_PV_PORMS_ALAP_dummy;

					for (int j = 1; j < ALAP_TASK_COUNT + 1; j++)
					{
						nor_Prom_time = generateTask->alap_dummy[j].promotion_time/generateTask->alap[j].promotion_time;
						avg_prom_time_dummy += nor_Prom_time;
					}
					avg_prom_time_dummy /= (double)(ALAP_TASK_COUNT);
					tot_prom_time_dummy += avg_prom_time_dummy;
				}	
#endif
				//SIMULATION#3 PORMS--noDUMMY, withSLACK **************
#ifdef use_slack
				RTSchedule->PV_PORMS[0] = 0.0;
				for(int pv = 1; pv < TOTAL_TASKS + 1; pv++)
				{
					RTSchedule->PV_PORMS[pv] = 0.0;
				}
				RTSchedule->PORMS_schedule_point_num = 0.0;
				RTSchedule->porms_taskset_area = 0.0;
				RTSchedule->porms_taskset_area_point = 0.0;

				PORMS_start_timer_slack = GetTimeMs64();
				schedulable = RTSchedule->runPORMS(highReadyQueue, lowReadyQueue, arrivalQueue, lcm, generateTask, withSlack);
				PORMS_finish_timer_slack = GetTimeMs64();
				if (schedulable == -1) {
					DTtrace << "@@@@@@@@@ " << i << "th task for PORMS was not schedulable @@@@@@@@@\n\n" << endl;
					i--;
					tot_time_PORMS -= one_time_PORMS;
					tot_time_RMS -= one_time_RMS;
					tot_time_PORMS_dummy -= one_time_PORMS_dummy;
					avg_schedule_time_PORMS -= one_time_PORMS / (double)PORMS_schedule_point_num;
					avg_schedule_time_RMS -= one_time_RMS / (double)RMS_schedule_point_num;
					avg_schedule_time_PORMS_dummy -= one_time_PORMS_dummy / (double)PORMS_schedule_point_num_dummy;
					tot_PORMS_schedule_point_num -= PORMS_schedule_point_num;
					tot_RMS_schedule_point_num -= RMS_schedule_point_num;
					tot_PORMS_schedule_point_num_dummy -= PORMS_schedule_point_num_dummy;
					tot_area = 0;
					tot_area_rms = 0;
					tot_area_dummy = 0;
					tot_PV_PORMS -= avg_PV_PORMS;
					tot_PV_PORMS_ASAP -= avg_PV_PORMS_ASAP;
					tot_PV_PORMS_ALAP -= avg_PV_PORMS_ALAP;
					tot_PV_RMS -= avg_PV_RMS;
					tot_PV_RMS_ASAP -= avg_PV_RMS_ASAP;
					tot_PV_RMS_ALAP -= avg_PV_RMS_ALAP;
					delete[] generateTask->asap;
					delete[] generateTask->alap;
					delete[] generateTask->tasks;
					continue;
				}

				//generateTask->printAllTasks(i);

				double one_time_PORMS_slack = PORMS_finish_timer_slack - PORMS_start_timer_slack;
				avg_schedule_time_PORMS_slack += one_time_PORMS_slack / (double)RTSchedule->PORMS_schedule_point_num;
				tot_time_PORMS_slack += one_time_PORMS_slack;
				PORMS_schedule_point_num_slack = RTSchedule->PORMS_schedule_point_num;
				tot_PORMS_schedule_point_num_slack += PORMS_schedule_point_num_slack;
				tot_area_slack = RTSchedule->porms_taskset_area;

				//************ CALCULATE RESULTS for PORMS--noDUMMY, withSLACK *************
				double avg_PV_PORMS_slack = 0;
				double avg_PV_PORMS_ASAP_slack = 0;
				double avg_PV_PORMS_ALAP_slack = 0;
				double avg_prom_time_slack = 0;

				for (int j = 1; j < TOTAL_TASKS + 1; j++)
				{
					RTSchedule->PV_PORMS[j] /= ((double)lcm/generateTask->task_periods[j - 1]);
					avg_PV_PORMS_slack += RTSchedule->PV_PORMS[j];
					if (generateTask->tasks[j - 1].preference_level == 1)
					{
						avg_PV_PORMS_ASAP_slack += RTSchedule->PV_PORMS[j];
					}
					else
					{
						avg_PV_PORMS_ALAP_slack += RTSchedule->PV_PORMS[j];
					}
				}

				avg_PV_PORMS_slack /= (double)(TOTAL_TASKS);
				avg_PV_PORMS_ASAP_slack /= (double)(ASAP_TASK_COUNT);
				avg_PV_PORMS_ALAP_slack /= (double)(ALAP_TASK_COUNT);
				tot_PV_PORMS_slack += avg_PV_PORMS_slack;
				tot_PV_PORMS_ASAP_slack += avg_PV_PORMS_ASAP_slack;
				tot_PV_PORMS_ALAP_slack += avg_PV_PORMS_ALAP_slack;
#endif
				//generateTask->printAllTasks(i);

				area_ratio += tot_area;
				area_ratio_rms += tot_area_rms;
				area_ratio_dummy += tot_area_dummy;
				//area_ratio_slack += tot_area_slack;

				delete[] generateTask->asap;
				delete[] generateTask->alap;
				delete[] generateTask->tasks;

				free(generateTask->task_wcet);
				free(generateTask->task_exet);
				free(generateTask->task_periods);
				free(generateTask->task_util);

				delete generateTask;
				delete RTSchedule;
				delete dummy_task;
			}

			avg_area_ratio = area_ratio_rms/area_ratio;
			avg_area_ratio_dummy = area_ratio_rms/area_ratio_dummy;
			//avg_area_ratio_slack = area_ratio_rms/area_ratio_slack;

#ifdef debug_print
			cout_trace << "\n\n||******************** Statistics **********************||\n" << endl;
			cout_trace << "avg_schedule_time_PORMS: " << avg_schedule_time_PORMS/(double)TASK_SETS_COUNT << " nsec" << endl;
			cout_trace << "avg_schedule_time_RMS: " << avg_schedule_time_RMS/(double)TASK_SETS_COUNT << " nsec" << endl;

			cout_trace << "\ntot_time_PORMS: " << tot_time_PORMS/(double)TASK_SETS_COUNT << " nsec" << endl;
			cout_trace << "tot_time_RMS: " << tot_time_RMS/(double)TASK_SETS_COUNT << " nsec" << endl;

			cout_trace <<"avg_PV_PORMS: "<< tot_PV_PORMS/(double)TASK_SETS_COUNT << endl;
			cout_trace <<"avg_PV_RMS: "<< tot_PV_RMS/(double)TASK_SETS_COUNT << endl;

			cout_trace << "\nnor_PV_PORMS: " << tot_PV_PORMS/tot_PV_RMS << endl;
#endif

			genPV_noDummy << ASAP_TASK_UTILIZATION << "\t" << avg_area_ratio << "\t" 
						  << tot_PV_PORMS/tot_PV_RMS << "\t"
						  << tot_PV_PORMS_ASAP/ tot_PV_RMS_ASAP << "\t" 
    					  << tot_PV_PORMS_ALAP/ tot_PV_RMS_ALAP << "\t\t\t" 
						  << tot_PORMS_schedule_point_num/TASK_SETS_COUNT << "\t"
						  << tot_RMS_schedule_point_num/TASK_SETS_COUNT << "\t"
						  << avg_schedule_time_PORMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						  << tot_time_PORMS/(double)TASK_SETS_COUNT << " nsec" <<"\t\t" 
						  << avg_schedule_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						  << tot_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t"
						  << tot_time_PORMS/tot_time_RMS << "\t"
						  << avg_schedule_time_PORMS/avg_schedule_time_RMS << endl;

#ifdef use_slack
			genPV_noSlack << ASAP_TASK_UTILIZATION << "\t" << avg_area_ratio << "\t" 
							<< tot_PV_PORMS/tot_PV_RMS << "\t"
							<< tot_PV_PORMS_ASAP/ tot_PV_RMS_ASAP << "\t" 
							<< tot_PV_PORMS_ALAP/ tot_PV_RMS_ALAP << "\t\t\t" 
							<< tot_PORMS_schedule_point_num/TASK_SETS_COUNT << "\t" 
						    << tot_RMS_schedule_point_num/TASK_SETS_COUNT << "\t"
							<< avg_schedule_time_PORMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
							<< tot_time_PORMS/(double)TASK_SETS_COUNT << " nsec" <<"\t\t" 
							<< avg_schedule_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
							<< tot_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t"
							<< tot_time_PORMS/tot_time_RMS << "\t"
							<< avg_schedule_time_PORMS/avg_schedule_time_RMS << endl;

			double avg_prom_time_slack = tot_prom_time_slack/(double)TASK_SETS_COUNT;
			genPV_Slack << ASAP_TASK_UTILIZATION << "\t" << avg_area_ratio_slack << "\t" 
						<< tot_PV_PORMS_slack/tot_PV_RMS << "\t"
						<< tot_PV_PORMS_ASAP_slack/tot_PV_RMS_ASAP << "\t" 
						<< tot_PV_PORMS_ALAP_slack/ tot_PV_RMS_ALAP << "\t\t\t" 
						<< tot_PORMS_schedule_point_num_slack/TASK_SETS_COUNT << "\t"
						<< tot_RMS_schedule_point_num/TASK_SETS_COUNT << "\t"
						<< avg_schedule_time_PORMS_slack/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						<< tot_time_PORMS_slack/(double)TASK_SETS_COUNT << " nsec" <<"\t\t" 
						<< avg_schedule_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						<< tot_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t"
						<< tot_time_PORMS_slack/tot_time_RMS << "\t"
						<< avg_schedule_time_PORMS_slack/avg_schedule_time_RMS << endl;
#endif

#ifdef use_dummy

#ifdef debug_print
			cout_trace << "avg_schedule_time_PORMS_dummy: " << avg_schedule_time_PORMS_dummy/(double)TASK_SETS_COUNT << " nsec" << endl;
			cout_trace << "\ntot_time_PORMS_dummy: " << tot_time_PORMS_dummy/(double)TASK_SETS_COUNT << " nsec" << endl;
			cout_trace << "avg_schedule_time_PORMS_dummy: " << avg_schedule_time_PORMS_dummy/(double)TASK_SETS_COUNT << " nsec" << endl;
			cout_trace << "\nnor_PV_PORMS_dummy: " << tot_PV_PORMS_dummy/tot_PV_RMS << endl;
#endif

			double avg_prom_time = tot_prom_time_dummy/(double)TASK_SETS_COUNT;
			genPV_Dummy << ASAP_TASK_UTILIZATION << "\t" << avg_area_ratio_dummy << "\t" 
						<< tot_PV_PORMS_dummy/tot_PV_RMS << "\t"
						<< tot_PV_PORMS_ASAP_dummy/tot_PV_RMS_ASAP << "\t" 
						<< tot_PV_PORMS_ALAP_dummy/ tot_PV_RMS_ALAP << "\t"
						<< avg_prom_time << "\t\t\t" 
						<< tot_PORMS_schedule_point_num_dummy/TASK_SETS_COUNT << "\t"
						<< tot_RMS_schedule_point_num/TASK_SETS_COUNT << "\t"
						<< avg_schedule_time_PORMS_dummy/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						<< tot_time_PORMS_dummy/(double)TASK_SETS_COUNT << " nsec" <<"\t" 
						<< avg_schedule_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t" 
						<< tot_time_RMS/(double)TASK_SETS_COUNT << " nsec" << "\t"
						<< tot_time_PORMS_dummy/tot_time_RMS << "\t"
						<< avg_schedule_time_PORMS_dummy/avg_schedule_time_RMS  << endl;
#endif
			//cout_trace << "\n\n||****************************************************||\n" << endl;
		}
	}
	//}
	//}

	genPV_noDummy.close();
	tracePV_noDummy.close();	
	genPV_Dummy.close();		
	tracePV_Dummy.close();
	return 0;
}

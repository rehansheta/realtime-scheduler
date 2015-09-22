#include <iostream>
#include <fstream>
#include <queue>
#include <math.h>


#include "Utility.h"
#include "TaskSetGenerator.h"
#include "Constants.h"


using namespace std;

extern int ASAP_TASK_COUNT;
extern int ALAP_TASK_COUNT;
extern double ASAP_TASK_UTILIZATION;
extern double ALAP_TASK_UTILIZATION;

extern double TOTAL_UTILIZATION;
extern int TOTAL_TASKS;

extern double MIN_U;
extern double MAX_U;

extern ofstream cout_trace;
extern ofstream tracePV_noDummy;
extern ofstream tracePV_Dummy;	
extern ofstream tracePV_noSlack;
extern ofstream tracePV_Slack;

CTaskSetGenerator::CTaskSetGenerator()
{

}

CTasks* CTaskSetGenerator::asap;
CTasks* CTaskSetGenerator::alap;
CTasks* CTaskSetGenerator::alap_dummy;
CTasks* CTaskSetGenerator::tasks;


CTaskSetGenerator::~CTaskSetGenerator()
{
	/*delete[] asap;
	delete[] alap;
	delete[] tasks;

	free(task_wcet);
	free(task_exet);
	free(task_periods);
	free(task_util);*/
}


//void CTaskSetGenerator::generatePORMSTaskSet()
//{		
//
//	// create all asap+alap tasks
//	asap = new CTasks[ASAP_TASK_COUNT];
//	alap = new CTasks[ALAP_TASK_COUNT];
//
//	for (int i = 0; i < ASAP_TASK_COUNT; i++) 
//	{
//		asap[i] = *(new CTasks(task_periods[i], task_wcet[i], (i + 1), 1));
//	}
//
//	for (int i = 0; i < ALAP_TASK_COUNT; i++) 
//	{
//		alap[i] = *(new CTasks(task_periods[i + ASAP_TASK_COUNT], task_wcet[i + ASAP_TASK_COUNT], (ASAP_TASK_COUNT + i + 1), 0));
//	}
//
//	// calculate the promotion time for all alap tasks
//	for (int i = 0; i < ALAP_TASK_COUNT; i++) {
//		alap[i].getPromotionTime(asap, alap);
//		cout_trace << "Promotion time of alapID:" << alap[i].id << " " << alap[i].promotion_time << endl;
//	}
//}

void CTaskSetGenerator::printAllTasks(int i)
{
#ifdef debug_print_taskSet
	tracePV_noSlack << "| high |" << endl;
	tracePV_Slack << "| high |" << endl;
	for (int j = 1; j < ASAP_TASK_COUNT + 1; j++)
	{
		tracePV_noSlack << "Task " << this->asap[j].id << ", WCET = "<<this->asap[j].wcet<<", period = " << this->asap[j].period \
			<<", deadline = "<< this->asap[j].deadline << ", pref_level = " << this->asap[j].preference_level << endl;
		tracePV_Slack << "Task " << this->asap[j].id << ", WCET = "<<this->asap[j].wcet<<", period = " << this->asap[j].period \
			<<", deadline = "<< this->asap[j].deadline << ", pref_level = " << this->asap[j].preference_level << endl;
	}
	tracePV_noSlack << "| low |" << endl;
	tracePV_Slack << "| low |" << endl;
	for (int j = 1; j < ALAP_TASK_COUNT + 1; j++)
	{
		tracePV_noSlack <<"Task "<< this->alap[j].id <<", WCET = "<<this->alap[j].wcet<<", period = "<<this->alap[j].period \
			<<", prom_time = "<< this->alap[j].promotion_time <<", deadline = "<< this->alap[j].deadline <<", pref_level = "<<this->alap[j].preference_level<<endl;
		tracePV_Slack <<"Task "<< this->alap[j].id <<", WCET = "<<this->alap[j].wcet<<", period = "<<this->alap[j].period \
			<<", prom_time = "<< this->alap[j].promotion_time <<", deadline = "<< this->alap[j].deadline <<", pref_level = "<<this->alap[j].preference_level<<endl;
	}
#ifdef use_dummy
	if (TOTAL_UTILIZATION < MAX_RMS_LOAD)
	{
		tracePV_Dummy << "Task " << this->asap[0].id << ", WCET = "<<this->asap[0].wcet<<", period = " << this->asap[0].period \
			<<", deadline = "<< this->asap[0].deadline << ", pref_level = " << this->asap[0].preference_level << endl;

		tracePV_Dummy << "| low dummy |" << endl;
		for (int j = 1; j < ALAP_TASK_COUNT + 1; j++)
		{
			tracePV_Dummy <<"Task "<< this->alap_dummy[j].id <<", WCET = "<<this->alap_dummy[j].wcet<<", period = "<<this->alap_dummy[j].period \
				<<", prom_time = "<< this->alap_dummy[j].promotion_time <<", deadline = "<< this->alap_dummy[j].deadline <<", pref_level = "<<this->alap_dummy[j].preference_level<<endl;
		}
	}
#endif

	/*tracePV_noSlack << "| single |" << endl;
	for (int j = 0; j < TOTAL_TASKS; j++)
	{
		tracePV_noSlack<<"Task "<< (j + 1) <<", WCET = "<<this->tasks[j].wcet<<", period = "<<this->tasks[j].period \
			<<", id = "<<this->tasks[j].id<<", pref_level = "<<this->tasks[j].preference_level<<endl;
		tracePV_Slack<<"Task "<< (j + 1) <<", WCET = "<<this->tasks[j].wcet<<", period = "<<this->tasks[j].period \
			<<", id = "<<this->tasks[j].id<<", pref_level = "<<this->tasks[j].preference_level<<endl;
	}*/
#endif
}


/* Random generate the period for tasks in the task set.
* succeed, return 1; Otherwise (LCM overflow), while loop until succeed
*/
double CTaskSetGenerator::GeneratePORMSTaskSet(double dyn_load)
{		
	double min_period = MAX_PERIOD + 1;
	long long tmp_p, orig_lcm;

	LCM = 1;

	// create all asap+alap tasks
	asap = new CTasks[ASAP_TASK_COUNT + 1];	//0th one is for the dummy task
	alap = new CTasks[ALAP_TASK_COUNT + 1];

	task_wcet = (double*) malloc(TOTAL_TASKS * sizeof(double));
	task_exet = (double*) malloc(TOTAL_TASKS * sizeof(double));
	task_periods = (double*) malloc(TOTAL_TASKS * sizeof(double));
	task_util = (double*) malloc(TOTAL_TASKS * sizeof(double));

	//generate task utilization for high preference tasks
	UUniFast(ASAP_TASK_COUNT, ASAP_TASK_UTILIZATION, 0);

	//generate task utilization for low preference tasks
	UUniFast(ALAP_TASK_COUNT, ALAP_TASK_UTILIZATION, ASAP_TASK_COUNT);

	asap[0] = *(new CTasks());
	for (int i = 1; i < ASAP_TASK_COUNT + 1; i++) 
	{
		tmp_p = myrandom(MIN_PERIOD, MAX_PERIOD);
		orig_lcm = LCM;

		//while(GetLCM(tmp_p) >= 4e9)
		//max: 2e7, for simulation speed, can take smaller number
		while((LCM = getLCM(LCM, tmp_p)) && LCM >= 2e5) 
		{
			tmp_p = myrandom(MIN_PERIOD, MAX_PERIOD);
			LCM = orig_lcm;
		}

		asap[i] = *(new CTasks(tmp_p, tmp_p * task_util[i - 1], (tmp_p * task_util[i - 1]) * dyn_load, i, 1));
		task_periods[i - 1] = tmp_p;
		task_wcet[i - 1] = tmp_p * task_util[i - 1];

		//TODO :: have to find a way to deal with this
		task_exet[i - 1] = task_wcet[i - 1] * dyn_load;

		if (task_periods[i - 1] < min_period)
		{
			min_period = task_periods[i - 1];
		}
	}

	alap[0] = *(new CTasks());
	for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) 
	{
		tmp_p = myrandom(MIN_PERIOD, MAX_PERIOD);
		orig_lcm = LCM;

		//while(GetLCM(tmp_p) >= 4e9)
		//max: 2e7, for simulation speed, can take smaller number
		while((LCM = getLCM(LCM, tmp_p)) && LCM >= 2e5) 
		{
			tmp_p = myrandom(MIN_PERIOD, MAX_PERIOD);
			LCM = orig_lcm;
		}
		
		alap[i] = *(new CTasks(tmp_p, tmp_p * task_util[i - 1 + ASAP_TASK_COUNT], (tmp_p * task_util[i - 1 + ASAP_TASK_COUNT]) * dyn_load, (ASAP_TASK_COUNT + i), 0));
		task_periods[i - 1 + ASAP_TASK_COUNT] = tmp_p;
		task_wcet[i - 1 + ASAP_TASK_COUNT] = tmp_p * task_util[i - 1 + ASAP_TASK_COUNT];

		//TODO :: have to find a way to deal with this
		task_exet[i - 1 + ASAP_TASK_COUNT] = task_wcet[i - 1 + ASAP_TASK_COUNT] * dyn_load;

		if (task_periods[i - 1 + ASAP_TASK_COUNT] < min_period)
		{
			min_period = task_periods[i - 1 + ASAP_TASK_COUNT];
		}
	}

	// calculate the promotion time for all alap tasks
	for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) {
		alap[i].getPromotionTime(asap, alap);
	}
	return min_period;
}

void CTaskSetGenerator::GeneratePORMSTaskSet_dummy(double dyn_load, double min_period)
{
	alap_dummy = new CTasks[ALAP_TASK_COUNT + 1];

	alap_dummy[0] = *(new CTasks());
	for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) 
	{
		alap_dummy[i] = *(new CTasks(alap[i].period, alap[i].wcet, alap[i].exet, alap[i].id, 0));
	}

	if (TOTAL_UTILIZATION < MAX_RMS_LOAD)
	{
		double dummy_util = (MAX_RMS_LOAD - TOTAL_UTILIZATION);
		double dummy_period = min_period;
		asap[0] = *(new CTasks(dummy_period, dummy_period * dummy_util, dummy_period * dummy_util, 0, 1));
		asap[0].is_dummy = 1;
	}

	// calculate the promotion time for all alap tasks
	for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) {
		alap_dummy[i].getPromotionTime_dummy(asap, alap_dummy);
		//cout_trace << "Promotion time of alapID:" << alap[i].id << " " << alap[i].promotion_time << endl;
	}
}



/*
* generate the utilization for each task using the idea in paper:
* "Biasing Effects in Schedulability Measures" -- Enrico Bini, Giorgio C. Buttazzo, ECRTS 2004
* */
void CTaskSetGenerator::UUniFast(int TaskNum, double Utot, int start_id)
{
	int n = TaskNum;
	double sumU = Utot;
	double nextsumU;
	double rand_number;
	int i, j;

	if (TaskNum == 1)
	{
		task_util[start_id] = sumU;
	}
	else
	{
		//check for parameter correctness
		if((n*MIN_U) > Utot || (n*MAX_U) < Utot){
			cout_trace <<"generation error. too low or too high"<<endl;
			exit(1);    
		}

		for(i = 0; i < n-1; i++)
		{
			rand_number = 0;
			while(rand_number == 0 || rand_number == 1)
			{
				rand_number = ((double)(rand()%1000))/1000.0;    
			}
			rand_number = pow(rand_number, 1.0/(double)(n-i-1));
			nextsumU = sumU*rand_number;

			//check for utilization limit
			if((sumU - nextsumU) >= MIN_U && (sumU - nextsumU) <= MAX_U)
			{
				//is it possible to generate the remaining tasks?
				if(((n-i-1)*MIN_U) >= nextsumU)
				{
					nextsumU = (n-i-1)*MIN_U;
					task_util[i+start_id] = (sumU - nextsumU);
					for(j = i+1; j < n; j++)
					{
						task_util[j+start_id] = MIN_U;    
					}
					break;
				}
				else if((n-i-1)*MAX_U <= nextsumU)
				{
					nextsumU = (n-i-1)*MAX_U;
					task_util[i+start_id] = (sumU - nextsumU);
					for(j = i+1; j < n; j++)
					{
						task_util[j+start_id] = MAX_U;    
					}
					break;
				}

				task_util[i+start_id] = (sumU - nextsumU);
				sumU = nextsumU;
			}
			else
			{
				//limit violated, ignore this one
				i = i - 1;
			}        
		}
		if(i == (n-1))
			task_util[i+start_id] = sumU;
	}

}


void CTaskSetGenerator::generateRMSTaskSet()
{
	// create all tasks
	tasks = new CTasks[TOTAL_TASKS];

	/*for (int i = 0; i < TOTAL_TASKS; i++) 
	{
		tasks[i] = *(new CTasks(task_periods[i], task_wcet[i], (i + 1), 1));
	}*/

	for (int i = 0; i < ASAP_TASK_COUNT; i++) 
	{
		tasks[i] = *(new CTasks(task_periods[i], task_wcet[i], task_exet[i], (i + 1), 1));
	}

	for (int i = 0; i < ALAP_TASK_COUNT; i++) 
	{
		tasks[i + ASAP_TASK_COUNT] = *(new CTasks(task_periods[i + ASAP_TASK_COUNT], task_wcet[i + ASAP_TASK_COUNT], task_exet[i + ASAP_TASK_COUNT], (ASAP_TASK_COUNT + i + 1), 0));
	}
}


int CTaskSetGenerator::getTaskSetLCM()
{
	int last_lcm, i;

	if(ASAP_TASK_COUNT + ALAP_TASK_COUNT < 2) return 0;

	last_lcm = getLCM(task_periods[0], task_periods[1]);

	for(i=2; i < ASAP_TASK_COUNT + ALAP_TASK_COUNT; i++)
	{
		last_lcm = getLCM(last_lcm, task_periods[i]);
	}

	return last_lcm;
}

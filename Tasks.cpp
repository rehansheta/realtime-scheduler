#include <iostream>
#include <queue>
#include <math.h>

#include "Tasks.h"
#include "Constants.h"

using namespace std;

extern double ASAP_TASK_UTILIZATION;
extern double ALAP_TASK_UTILIZATION;

extern double TOTAL_UTILIZATION;

extern double MIN_U;
extern double MAX_U;

extern int ASAP_TASK_COUNT;
extern int ALAP_TASK_COUNT;
extern ofstream cout_trace;


CTasks::CTasks()
{
}

CTasks::CTasks(double period, double wcet, double exet, int id, int pref_level)
{
	this->id = id;
	this->wcet = wcet;
	this->exet = exet;
	this->period = period;
	this->deadline = period;
	this->start_time = 0.0;
	this->original_start_time = 0.0;
	this->next_start_time = 0.0;
	this->remaining_time = this->exet;
	this->preference_level = pref_level;
	this->promoting = 0;
	this->is_dummy = 0;
	this->is_preempted = 0;
	this->pv_start = 0;
}

CTasks::~CTasks()
{
}
	
void CTasks::getPromotionTime(CTasks* asap, CTasks* alap)
{
	double period_length;
	double last_period_length;
	double temp;			

	period_length = 0;
	last_period_length = -1;

	// (period_length > this->deadline) && ...
	while ((fabs((period_length - this->deadline)) < EPS || period_length < this->deadline) && (period_length != last_period_length))
	{
		last_period_length = period_length;
		temp = 0;
		
		for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) 
		{	
			if(alap[i].deadline < this->deadline)	
			{		
				temp += ceil (last_period_length / alap[i].deadline) * alap[i].wcet;
			}
			else if (fabs(alap[i].deadline - this->deadline) < EPS && (alap[i].id < this->id)) 
			{
				temp += ceil (last_period_length / alap[i].deadline) * alap[i].wcet;
			}
		}

		for (int i = 1; i < ASAP_TASK_COUNT + 1; i++)
		{
			if(asap[i].deadline < this->deadline)	
			{
					temp += ceil (last_period_length / asap[i].deadline) * asap[i].wcet;
			}
			else if (fabs(asap[i].deadline - this->deadline) < EPS && (asap[i].id < this->id))
			{
				temp += ceil (last_period_length / asap[i].deadline) * asap[i].wcet;
			}
		}

		period_length = this->wcet + temp;

	}

	this->promotion_time = (this->deadline - last_period_length);
	this->original_promotion_time = (this->deadline - last_period_length);

	// added for new PV_value calculation
	this->next_start_time = 0;	//this->promotion_time;
	this->promoting = 0;
}

void CTasks::getPromotionTime_dummy(CTasks* asap, CTasks* alap_dummy)
{
	double period_length;
	double last_period_length;
	double temp;			

	period_length = 0;
	last_period_length = -1;

	// (period_length > this->deadline) && ...
	while ((fabs((period_length - this->deadline)) < EPS || period_length < this->deadline) && (period_length != last_period_length))
	{
		last_period_length = period_length;
		temp = 0;
		
		for (int i = 1; i < ALAP_TASK_COUNT + 1; i++) 
		{	
			if(alap_dummy[i].deadline < this->deadline)	
			{		
				temp += ceil (last_period_length / alap_dummy[i].deadline) * alap_dummy[i].wcet;
			}
			else if (fabs(alap_dummy[i].deadline - this->deadline) < EPS && (alap_dummy[i].id < this->id)) 
			{
				temp += ceil (last_period_length / alap_dummy[i].deadline) * alap_dummy[i].wcet;
			}
		}
		
		if (TOTAL_UTILIZATION < MAX_RMS_LOAD)
		{
			if(asap[0].deadline < this->deadline)	
			{
				temp += ceil (last_period_length / asap[0].deadline) * asap[0].wcet;
			}
			else if (fabs(asap[0].deadline - this->deadline) < EPS && (asap[0].id < this->id))
			{
				temp += ceil (last_period_length / asap[0].deadline) * asap[0].wcet;
			}
		}

		for (int i = 1; i < ASAP_TASK_COUNT + 1; i++)
		{
			if(asap[i].deadline < this->deadline)	
			{
					temp += ceil (last_period_length / asap[i].deadline) * asap[i].wcet;
			}
			else if (fabs(asap[i].deadline - this->deadline) < EPS && (asap[i].id < this->id))
			{
				temp += ceil (last_period_length / asap[i].deadline) * asap[i].wcet;
			}
		}

		period_length = this->wcet + temp;

	}

	this->promotion_time = (this->deadline - last_period_length);
	this->original_promotion_time = (this->deadline - last_period_length);

	// added for new PV_value calculation
	this->next_start_time = 0;	//this->promotion_time;
	this->promoting = 0;
}

void CTasks::calcPORMS_PV(double* PV_PORMS, double timeline)
{
	//std::cout.setf(std::ios::fixed);
    //std::cout.precision(10);
	
	double pv_current;
	if (this->preference_level == 1)   //high preference tasks
	{
		pv_current = ((this->period - this->finish_time) / (this->deadline - this->exet));
		PV_PORMS[this->id] += ((this->period - this->finish_time) / (this->deadline - this->exet));

#ifdef debug_print_PV_porms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_PORMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_PORMS: " << PV_PORMS[this->id] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.period - currTask.finish_time: " << this->period - this->finish_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif

	}
	else  //low preference tasks
	{	
		if (this->pv_start == 0)
		{
			pv_current = ((this->start_time - this->next_start_time) / (this->deadline - this->wcet));
			PV_PORMS[this->id] += ((this->start_time - this->next_start_time) / (this->deadline - this->wcet));

#ifdef debug_print_PV_porms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_PORMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_PORMS: " << PV_PORMS[this->id] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.promotion_time: " << this->promotion_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.original_start_time: " << this->original_start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.start_time - currTask.next_start_time: " << this->start_time - this->next_start_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif

		}
		else 
		{
			// THIS USES ORIGINAL START TIME
			pv_current = ((this->original_start_time - this->next_start_time) / (this->deadline - this->wcet));
			PV_PORMS[this->id] += ((this->original_start_time - this->next_start_time) / (this->deadline - this->wcet));
			this->pv_start = 0;

#ifdef debug_print_PV_porms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_PORMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_PORMS: " << PV_PORMS[this->id] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.promotion_time: " << this->promotion_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.original_start_time: " << this->original_start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.original_start_time - currTask.next_start_time: " << this->original_start_time - this->next_start_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif

		}
	}
}

void CTasks::calcRMS_PV(double* PV_RMS)
{
	//std::cout.setf(std::ios::fixed);
    //std::cout.precision(10);
	
	double pv_current;
	if (this->preference_level == 1)   //high preference tasks
	{
		pv_current = ((this->period - this->finish_time) / (this->deadline - this->exet));
		PV_RMS[this->id - 1] += ((this->period - this->finish_time) / (this->deadline - this->exet));

#ifdef debug_print_PV_rms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_RMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_RMS: " << PV_RMS[this->id - 1] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.period - currTask.finish_time: " << this->period - this->finish_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif

	}
	else  //low preference tasks
	{	
		if (this->pv_start == 0)
		{
			pv_current = ((this->start_time - this->next_start_time) / (this->deadline - this->wcet));
			PV_RMS[this->id - 1] += ((this->start_time - this->next_start_time) / (this->deadline - this->wcet));

#ifdef debug_print_PV_rms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_RMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_RMS: " << PV_RMS[this->id - 1] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.original_start_time: " << this->original_start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.start_time - currTask.next_start_time: " << this->start_time - this->next_start_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif
		}
		else 
		{
			//THIS USES ORIGINAL START TIME
			pv_current = ((this->original_start_time - this->next_start_time) / (this->deadline - this->wcet));
			PV_RMS[this->id - 1] += ((this->original_start_time - this->next_start_time) / (this->deadline - this->wcet));
			this->pv_start = 0;

#ifdef debug_print_PV_rms
	cout_trace << "taskID: "<< this->id << endl;
	cout_trace << "preference-level: " << this->preference_level << endl;
	cout_trace << "Current_PV_RMS: " << pv_current << endl;
	cout_trace << "Cumulative_PV_RMS: " << PV_RMS[this->id - 1] << endl;
	cout_trace << "currTask.period: " << this->period << endl;
	cout_trace << "currTask.next_start_time: " << this->next_start_time << endl;
	cout_trace << "currTask.start_time: " << this->start_time << endl;
	cout_trace << "currTask.original_start_time: " << this->original_start_time << endl;
	cout_trace << "currTask.finish_time: " << this->finish_time << endl;
	cout_trace << "currTask.original_start_time - currTask.next_start_time: " << this->original_start_time - this->next_start_time << endl;
	cout_trace << "currTask.deadline: " << this->deadline << endl;
	cout_trace << "currTask.exet: " << this->exet << endl;
	cout_trace << "currTask.deadline - currTask.exet: " << this->deadline - this->exet << endl;
	cout_trace << endl;
#endif

		}
	}
}

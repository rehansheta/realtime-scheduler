#include <iostream>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <math.h>

#include "RTScheduling.h"
#include "QComparators.h"
#include "Constants.h"
#include "TaskSetGenerator.h"
#include "Tasks.h"
#include "Slack.h"
#include <assert.h>

using namespace std;

extern int ASAP_TASK_COUNT;
extern int ALAP_TASK_COUNT;
extern double ASAP_TASK_UTILIZATION;
extern double ALAP_TASK_UTILIZATION;

extern double TOTAL_UTILIZATION;
extern int TOTAL_TASKS;

extern double MIN_U;
extern double MAX_U;

extern ofstream DTtrace;
extern ofstream cout_trace;
extern ofstream tracePV_noSlack;
extern ofstream tracePV_Slack;

double CRTScheduling::porms_taskset_area_point;
double CRTScheduling::porms_taskset_area;
double CRTScheduling::rms_taskset_area_point;
double CRTScheduling::rms_taskset_area;

double* CRTScheduling::PV_PORMS;
double* CRTScheduling::PV_RMS;

CRTScheduling::CRTScheduling()
{
	/*create the PV value for each task in each scheme*/
	PV_PORMS = new double[TOTAL_TASKS + 1];
	PV_RMS = new double[TOTAL_TASKS];

	PV_PORMS[0] = 0.0;
	for(int i = 1; i < TOTAL_TASKS + 1; i++)
	{
		PV_PORMS[i] = 0.0;
		PV_RMS[i - 1] = 0.0;
	}

	/*initialize POEDF_schedule_point_num value*/
	PORMS_schedule_point_num = 0;
	RMS_schedule_point_num = 0;

	porms_taskset_area_point = 0.0;
	porms_taskset_area = 0.0;
	rms_taskset_area_point = 0.0;
	rms_taskset_area = 0.0;
}

CRTScheduling::~CRTScheduling()
{
	delete[] PV_PORMS;
	delete[] PV_RMS;
}

double CRTScheduling::getTaskSetPORMSArea(double finish_time, int task_level)
{
	//cout_trace << "\n\n~~~~~~~~AREA UNDER PORMS~~~~~~~~" << endl;

	double from = this->porms_taskset_area_point;
	this->porms_taskset_area_point = finish_time;
	double to = this->porms_taskset_area_point;

	if (task_level == 1)
	{
		this->porms_taskset_area += (pow ((to - from), 2.0) / 2.0) + ((to -from) * (this->LCM_for_area - to));
	}

	return 0;
}

double CRTScheduling::getTaskSetRMSArea(double finish_time, int task_level)
{
	//cout_trace << "\n\n~~~~~~~~AREA UNDER RMS~~~~~~~~" << endl;

	double from = this->rms_taskset_area_point;
	this->rms_taskset_area_point = finish_time;
	double to = this->rms_taskset_area_point;
	if (task_level == 1)
	{
		this->rms_taskset_area += (pow ((to - from), 2.0) / 2.0) + ((to -from) * (this->LCM_for_area - to));
	}

	//tracePV_Slack << "from: " << from << endl;
	//tracePV_Slack << "to: " << to << endl;
	//tracePV_Slack << "this->rms_taskset_area: " << this->rms_taskset_area << endl;
	return 0;
}

void CRTScheduling::printTasks(double timeline, PQV<CTasks, CPeriodComparator> highReadyQueue, 
					PQV<CTasks, CPromotionComparator> lowReadyQueue,
					PQV<CTasks, CArrivalComparator> arrivalQueue)
{
#ifdef debug_print
	cout_trace << "\n\n|| TIMELINE: " << timeline << " ||\n" << endl;
		
	if (!highReadyQueue.empty())
	{
		CTasks currTask = highReadyQueue.top();
		cout_trace << "currTask.id: " << currTask.id << endl;
		cout_trace << "currTask.required_time: " << currTask.remaining_time << endl;
		cout_trace << "currTask.deadline: " << currTask.period << endl;
	}

	if (!lowReadyQueue.empty())
	{
		CTasks promTask = lowReadyQueue.top();
		cout_trace << "promTask.id: " << promTask.id << endl;
		cout_trace << "promTask.promotion_time: " << promTask.promotion_time << endl;
		cout_trace << "promTask.required_time: " << promTask.remaining_time << endl;
		cout_trace << "promTask.deadline: " << promTask.period << endl;
	}

	if (!arrivalQueue.empty())
	{
		CTasks arrivTask = arrivalQueue.top();
		cout_trace << "arrivTask.id: " << arrivTask.id << endl;
		cout_trace << "arrivTask.next_start_time: " << arrivTask.next_start_time << endl;
		cout_trace << "arrivTask.required_time: " << arrivTask.remaining_time << endl;
		cout_trace << "arrivTask.deadline: " << arrivTask.period << endl;
	}
#endif
}

void CRTScheduling::printPreemptions(double timeline, int preemption_type, CTasks currTask)
{
#ifdef debug_print
	if (preemption_type == 1)
	{
		cout_trace << "\nPREEMPTION #1\n" << endl;
		cout_trace << "executed task ID: " << currTask.id << endl;
		cout_trace << "start time: " << currTask.start_time << endl;
		cout_trace << "  executing......" << endl;
		cout_trace << "expected finish time: " << currTask.finish_time << endl;
		cout_trace << "actual finish time: " << timeline << endl;
	}
	else if (preemption_type == 2)
	{
		cout_trace << "\nPREEMPTION #2\n" << endl;
	}
	else if (preemption_type == 3)
	{
		cout_trace << "\nPREEMPTION #3\n" << endl;
		cout_trace << "executed task ID: " << currTask.id << endl;
		cout_trace << "start time: " << currTask.start_time << endl;
		cout_trace << "  executing......" << endl;
		cout_trace << "finish time: " << currTask.finish_time << endl;
	}
	else
	{
		cout_trace << "\nNO PREEMPTION\n" << endl;
		cout_trace << "executed task ID: " << currTask.id << endl;
		cout_trace << "start time: " << currTask.start_time << endl;
		cout_trace << "  executing......" << endl;
		cout_trace << "finish time: " << currTask.finish_time << endl;
	}
#endif
}

void CRTScheduling::preempt(PQV<CTasks, CPeriodComparator> *highReadyQueue, 
					PQV<CTasks, CPromotionComparator> *lowReadyQueue,
					PQV<CTasks, CArrivalComparator> *arrivalQueue, int preemptType)
{
	if (preemptType == 1)
	{
		CTasks arrivTask = arrivalQueue->top();

		arrivalQueue->pop();
		if ((arrivTask.preference_level == 0) && (arrivTask.promotion_time > 0))
		{
#ifdef debug_print
			cout << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO LOW" << endl;
#endif
			lowReadyQueue->push(arrivTask);
		}
		else
		{
#ifdef debug_print	
			cout << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
			highReadyQueue->push(arrivTask);
		}
	}
	else if (preemptType == 0)
	{
		CTasks promTask = lowReadyQueue->top();
#ifdef debug_print
		cout << "PUSHING PROM TASK ID: " << promTask.id << " TO HIGH" << endl;
#endif	
		lowReadyQueue->pop();
		highReadyQueue->push(promTask);
	}
	else
		cout << "ERROR!" << endl;
}

void CRTScheduling::addNewSlack(PQV <CSlack, CSlackComparator> *slackQueue, CTasks top)
{
	CSlack* slack_task = new CSlack();					
	slack_task->slack_value = top.wcet - top.exet;
	slack_task->slack_deadline = top.deadline;
	slack_task->slack_id = top.id;
	slack_task->slack_preference_level = top.preference_level;

	slackQueue->push(*slack_task);
	delete slack_task;
}

void CRTScheduling::pushForwardSlack(PQV <CSlack, CSlackComparator> *slackQueue, double slack_value, CTasks top)
{
	CSlack *slack_top = &((*slackQueue)[slackQueue->size() - 1]);

	CSlack* slack_task = new CSlack();					
	slack_task->slack_value = slack_value;
	slack_task->slack_deadline = top.deadline;
	slack_task->slack_id = top.id;
	slack_task->slack_preference_level = top.preference_level;

	if (fabs(slack_top->slack_value - 0.0) < EPS)
	{
		slackQueue->pop();
	}

	if (slack_task->slack_value > 0.0)
	{
		slackQueue->push(*slack_task);
	}
	delete slack_task;
}

void CRTScheduling::addDummy(PQV <CTasks, CPeriodComparator> *highReadyQueue, 
							 PQV <CTasks, CArrivalComparator> *arrivalQueue, 
							 PQV <CSlack, CSlackComparator> *slackQueue, CTasks* currTask)
{
	CSlack* dummy_slack_task = new CSlack();					
	dummy_slack_task->slack_value = currTask->wcet;
	dummy_slack_task->slack_deadline = currTask->deadline;
	dummy_slack_task->slack_id = currTask->id;
	dummy_slack_task->slack_preference_level = currTask->preference_level;

	slackQueue->push(*dummy_slack_task);

	currTask->next_start_time = currTask->period;
	currTask->period = currTask->period + currTask->deadline;
	currTask->remaining_time = currTask->exet;
	arrivalQueue->push(*currTask);
	highReadyQueue->pop();

	delete dummy_slack_task;
}

void CRTScheduling::consumeSlack(PQV <CSlack, CSlackComparator> *slackQueue, double idle_time)
{
	for (unsigned itr = 0; itr < slackQueue->size();) 
	{
		CSlack *slack_top = &((*slackQueue)[slackQueue->size() - itr - 1]);
		if (idle_time > 0.0)
		{
			if (slack_top->slack_value > idle_time)
			{
				slack_top->slack_value = slack_top->slack_value - idle_time;
				if (fabs(slack_top->slack_value - 0.0) < EPS)
				{
					slackQueue->pop();
				}
				break;
			}
			else
			{
				idle_time = idle_time - slack_top->slack_value;
				slack_top->slack_value = 0.0;
				slackQueue->pop();
			}
		}
		else
		{
			break;
		}
	}
}

int CRTScheduling::manageSlack(PQV <CTasks, CPeriodComparator> *highReadyQueue,
							 PQV <CSlack, CSlackComparator> *slackQueue,
							 PQV <CTasks, CPromotionComparator> *lowReadyQueue,
							 PQV <CTasks, CArrivalComparator> *arrivalQueue,
							 double preemptTime, int preemptType, double* timeline, int lcm)
{
	CSlack *slack_top = &((*slackQueue)[slackQueue->size() - 1]);
	CTasks *top = &((*highReadyQueue)[highReadyQueue->size() - 1]);
	CTasks promTask;	// = *(new CTasks());
	CTasks arrivTask;	// = *(new CTasks());

	if (!lowReadyQueue->empty())
	{
		promTask = lowReadyQueue->top();
	}

	if (!arrivalQueue->empty())
	{
		arrivTask = arrivalQueue->top();
	}

	//case:: top_priority > slack_priority && task is ALAP/ASAP ---> execute top task + pass the slack
	if ((top->deadline < slack_top->slack_deadline) || ((fabs(top->deadline - slack_top->slack_deadline) < EPS) && (top->id <= slack_top->slack_id)))
	{
		top->start_time = *timeline;
		if (top->is_preempted == 1)
		{
			top->finish_time = *timeline + top->remaining_time;
			top->is_preempted = 0;
		}
		else
		{
			top->finish_time = *timeline + top->remaining_time;
		}

		// 1. Preempted: promo_time is in between s_time and f_time
		if ((preemptTime > top->start_time) && (preemptTime < top->finish_time))
		{
			top->remaining_time = top->finish_time - preemptTime;
			if (top->pv_start == 0)
			{
				top->original_start_time = top->start_time;
				top->pv_start = 1;
			}
			top->is_preempted = 1;
			double executed_time = preemptTime - top->start_time;
			*timeline = *timeline + executed_time;

			if ((top->remaining_time > (lcm - *timeline)) || (*timeline > lcm) || (fabs(lcm - *timeline) < EPS))
			{
				return NOT_SCHEDULABLE;
			}

			if (top->preference_level == 1) {
				this->getTaskSetPORMSArea(*timeline, 1);
			}
			else{
				this->getTaskSetPORMSArea(*timeline, 0);
			}

			preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
			PORMS_schedule_point_num++;
		}

		// 2. Preempted: promo_time is equal to s_time
		else if (fabs(preemptTime - top->start_time) < EPS)
		{
			preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
			PORMS_schedule_point_num++;
		}

		// 3. Preempted: promo_time is equal to f_time
		else if ((fabs(preemptTime - top->finish_time) < EPS) || (preemptTime > top->finish_time))
		{
			*timeline = top->finish_time;
			top->remaining_time = top->exet;

			if (top->finish_time > top->period)
			{
				return NOT_SCHEDULABLE;
			}

			top->calcPORMS_PV(PV_PORMS, *timeline);
			top->next_start_time = top->period;
			top->period = top->period + top->deadline; 

			highReadyQueue->pop();
			if (top->preference_level == 1) {
				this->getTaskSetPORMSArea(*timeline, 1);
				arrivalQueue->push(*top);
			}
			else {
				this->getTaskSetPORMSArea(*timeline, 0);
				top->promotion_time = top->deadline + top->promotion_time;
				//lowReadyQueue->push(*top);
				arrivalQueue->push(*top);
			}

			if (fabs(preemptTime - top->finish_time) < EPS)
			{
				preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
			}

			if (top->exet < top->wcet) 
			{
				addNewSlack(slackQueue, *top);
			}

			PORMS_schedule_point_num++;
		}
		else
		{
#ifdef debug_print
			cout_trace << "SLACK1:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
		}
	}
	//case: top_priority < slack_priority & task is ALAP ---> execute next ASAP tasks from queue + consume the slack
	//case: top_priority < slack_priority & task is ASAP ---> execute top ASAP task + consume the slack
	else
	{
		if (top->preference_level == 1)
		{
			if (top->remaining_time < slack_top->slack_value)
			{
				top->start_time = *timeline;
				if (top->is_preempted == 1)
				{
					top->finish_time = *timeline + top->remaining_time;
					top->is_preempted = 0;
				}
				else
				{
					top->finish_time = *timeline + top->remaining_time;
				}

				// 1. Preempted: promo_time is in between s_time and f_time
				if ((preemptTime > top->start_time) && (preemptTime < top->finish_time))
				{
					top->remaining_time = top->finish_time - preemptTime;
					if (top->pv_start == 0)
					{
						top->original_start_time = top->start_time;
						top->pv_start = 1;
					}
					top->is_preempted = 1;
					double executed_time = preemptTime - top->start_time;
					*timeline = *timeline + executed_time;
					slack_top->slack_value = slack_top->slack_value - executed_time;

					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, executed_time, *top);

					if ((top->remaining_time > (lcm - *timeline)) || (*timeline > lcm) || (fabs(lcm - *timeline) < EPS))
					{
						return NOT_SCHEDULABLE;
					}
					this->getTaskSetPORMSArea(*timeline, 1);

					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}

				// 2. Preempted: promo_time is equal to s_time
				else if (fabs(preemptTime - top->start_time) < EPS)
				{
					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}

				// 3. Preempted: promo_time is equal to f_time
				else if ((fabs(preemptTime - top->finish_time) < EPS) || (preemptTime > top->finish_time))
				{
					*timeline = top->finish_time;
					slack_top->slack_value = slack_top->slack_value - top->remaining_time;
					top->remaining_time = top->exet;

					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, top->remaining_time, *top);

					if (top->finish_time > top->period)
					{
						return NOT_SCHEDULABLE;
					}

					top->calcPORMS_PV(PV_PORMS, *timeline);
					top->next_start_time = top->period;
					top->period = top->period + top->deadline; 

					highReadyQueue->pop();
					arrivalQueue->push(*top);
					this->getTaskSetPORMSArea(*timeline, 1);

					if (fabs(preemptTime - top->finish_time) < EPS)
					{
						preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					}

					if (top->exet < top->wcet) 
					{
						addNewSlack(slackQueue, *top);
					}
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "SLACK2:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
				}
			}
			else if (top->remaining_time > slack_top->slack_value)
			{
				top->is_preempted = 1;
				top->start_time = *timeline;

				if (slack_top->slack_value < (preemptTime - top->start_time))
				{
					*timeline = *timeline + slack_top->slack_value;
					top->remaining_time = top->remaining_time - slack_top->slack_value;
					double slack_value = slack_top->slack_value;
					slack_top->slack_value = 0.0;

					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, slack_value, *top);
				}
				else if ((fabs(slack_top->slack_value - (preemptTime - top->start_time)) < EPS) || (slack_top->slack_value > (preemptTime - top->start_time)))
				{
					*timeline = preemptTime;
					top->remaining_time = top->remaining_time - (preemptTime - top->start_time);
					slack_top->slack_value = slack_top->slack_value - (preemptTime - top->start_time);
					
					this->getTaskSetPORMSArea(*timeline, 1);
					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, (preemptTime - top->start_time), *top);

					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "SLACK3:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
				}
			}
			else if (fabs(top->remaining_time - slack_top->slack_value) < EPS)
			{
				top->start_time = *timeline;
				if (top->is_preempted == 1)
				{
					top->finish_time = *timeline + top->remaining_time;
					top->is_preempted = 0;
				}
				else
				{
					top->finish_time = *timeline + top->remaining_time;
				}

				// check if the task can get preempted
				// 1. Preempted: promo_time is in between s_time and f_time
				if ((preemptTime > top->start_time) && (preemptTime < top->finish_time))
				{
					top->remaining_time = top->finish_time - preemptTime;
					if (top->pv_start == 0)
					{
						top->original_start_time = top->start_time;
						top->pv_start = 1;
					}
					top->is_preempted = 1;
					double executed_time = preemptTime - top->start_time;
					*timeline = *timeline + executed_time;
					slack_top->slack_value = slack_top->slack_value - executed_time;

					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, executed_time, *top);

					if ((top->remaining_time > (lcm - *timeline)) || (*timeline > lcm) || (fabs(lcm - *timeline) < EPS))
					{
						return NOT_SCHEDULABLE;
					}

					this->getTaskSetPORMSArea(*timeline, 1);

					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}

				// 2. Preempted: promo_time is equal to s_time
				else if (fabs(preemptTime - top->start_time) < EPS)
				{
					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}

				// 3. Preempted: promo_time is equal to f_time
				else if ((fabs(preemptTime - top->finish_time) < EPS) || (preemptTime > top->finish_time))
				{
					*timeline = *timeline + slack_top->slack_value;
					top->remaining_time = top->exet;
					double slack_value = slack_top->slack_value;
					slack_top->slack_value = 0.0;

					// ADD PUSH-FORWARD SLACK
					pushForwardSlack(slackQueue, slack_value, *top);

					if (top->finish_time > top->period)
					{
						return NOT_SCHEDULABLE;
					}

					top->calcPORMS_PV(PV_PORMS, *timeline);
					top->next_start_time = top->period;
					top->period = top->period + top->deadline; 

					highReadyQueue->pop();
					arrivalQueue->push(*top);
					this->getTaskSetPORMSArea(*timeline, 1);

					if (fabs(preemptTime - top->finish_time) < EPS)
					{
						preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					}

					if (top->exet < top->wcet) 
					{
						addNewSlack(slackQueue, *top);
					}
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "SLACK4:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
				}
			}
		}
		else
		{
			this->flag = 1;	// this this->flag is used to manage the 4th case of slack management
			for (unsigned itr = 1; itr < highReadyQueue->size();) 
			{
				CTasks *top = &((*highReadyQueue)[highReadyQueue->size() - itr - 1]);
				if (top->preference_level == 1 && slack_top->slack_value > 0)
				{
					if (top->remaining_time < slack_top->slack_value)
					{
						top->start_time = *timeline;
						if (top->is_preempted == 1)
						{
							top->finish_time = *timeline + top->remaining_time;
							top->is_preempted = 0;
						}
						else
						{
							top->finish_time = *timeline + top->remaining_time;
						}

						// check if the task can get preempted
						// 1. Preempted: promo_time is in between s_time and f_time
						if ((preemptTime > top->start_time) && (preemptTime < top->finish_time))
						{
							top->remaining_time = top->finish_time - preemptTime;
							if (top->pv_start == 0)
							{
								top->original_start_time = top->start_time;
								top->pv_start = 1;
							}
							top->is_preempted = 1;
							double executed_time = preemptTime - top->start_time;
							*timeline = *timeline + executed_time;
							slack_top->slack_value = slack_top->slack_value - executed_time;

							this->getTaskSetPORMSArea(*timeline, 1);
							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, executed_time, *top);

							if ((top->remaining_time > (lcm - *timeline)) || (*timeline > lcm) || (fabs(lcm - *timeline) < EPS))
							{
								return NOT_SCHEDULABLE;
							}

							preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
							//continue;
						}

						// 2. Preempted: promo_time is equal to s_time
						else if (fabs(preemptTime - top->start_time) < EPS)
						{
							preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
							//continue;
						}

						// 3. Preempted: promo_time is equal to f_time
						else if ((fabs(preemptTime - top->finish_time) < EPS) || (preemptTime > top->finish_time))
						{
							*timeline = top->finish_time;
							slack_top->slack_value = slack_top->slack_value - top->remaining_time;
							top->remaining_time = top->exet;

							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, top->remaining_time, *top);

							if (top->finish_time > top->period)
							{
								return NOT_SCHEDULABLE;
							}

							top->calcPORMS_PV(PV_PORMS, *timeline);
							top->next_start_time = top->period;
							top->period = top->period + top->deadline; 

							arrivalQueue->push(*top);
							highReadyQueue->erase(highReadyQueue->begin() + (highReadyQueue->size() - itr - 1));
							this->getTaskSetPORMSArea(*timeline, 1);

							if (fabs(preemptTime - top->finish_time) < EPS)
							{
								preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							}

							if (top->exet < top->wcet) 
							{
								addNewSlack(slackQueue, *top);
							}

							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
							//continue;
						}
						else
						{
#ifdef debug_print
							cout_trace << "SLACK5:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
						}
					}
					else if (top->remaining_time > slack_top->slack_value)
					{
						top->start_time = *timeline;
						top->is_preempted = 1;

						if (slack_top->slack_value < (preemptTime - top->start_time))
						{
							*timeline = *timeline + slack_top->slack_value;
							top->remaining_time = top->remaining_time - slack_top->slack_value;
							double slack_value = slack_top->slack_value;
							slack_top->slack_value = 0.0;

							this->getTaskSetPORMSArea(*timeline, 1);
							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, slack_value, *top);

							this->flag = 0;
							return 0;
						}
						else if ((fabs(slack_top->slack_value - (preemptTime - top->start_time)) < EPS) || (slack_top->slack_value > (preemptTime - top->start_time)))
						{
							*timeline = preemptTime;
							top->remaining_time = top->remaining_time - (preemptTime - top->start_time);
							slack_top->slack_value = slack_top->slack_value - (preemptTime - top->start_time);

							this->getTaskSetPORMSArea(*timeline, 1);
							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, (preemptTime - top->start_time), *top);

							preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
						}
						else
						{
#ifdef debug_print
							cout_trace << "SLACK5:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
						}
					}
					else if (fabs(top->remaining_time - slack_top->slack_value) < EPS)
					{
						top->start_time = *timeline;
						if (top->is_preempted == 1)
						{
							top->finish_time = *timeline + top->remaining_time;
							top->is_preempted = 0;
						}
						else
						{
							top->finish_time = *timeline + top->remaining_time;
						}

						// check if the task can get preempted
						// 1. Preempted: promo_time is in between s_time and f_time
						if ((preemptTime > top->start_time) && (preemptTime < top->finish_time))
						{
							top->remaining_time = top->finish_time - preemptTime;
							if (top->pv_start == 0)
							{
								top->original_start_time = top->start_time;
								top->pv_start = 1;
							}
							top->is_preempted = 1;
							double executed_time = preemptTime - top->start_time;
							*timeline = *timeline + executed_time;
							slack_top->slack_value = slack_top->slack_value - executed_time;

							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, executed_time, *top);

							if ((top->remaining_time > (lcm - *timeline)) || (*timeline > lcm) || (fabs(lcm - *timeline) < EPS))
							{
								return NOT_SCHEDULABLE;
							}
							this->getTaskSetPORMSArea(*timeline, 1);

							preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
							//continue;
						}

						// 2. Preempted: promo_time is equal to s_time
						else if (fabs(preemptTime - top->start_time) < EPS)
						{
							preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
							//continue;
						}

						// 3. Preempted: promo_time is equal to f_time
						else if ((fabs(preemptTime - top->finish_time) < EPS) || (preemptTime > top->finish_time))
						{
							*timeline = *timeline + slack_top->slack_value;
							top->remaining_time = top->exet;
							double slack_value = slack_top->slack_value;
							slack_top->slack_value = 0.0;

							// ADD PUSH-FORWARD SLACK
							pushForwardSlack(slackQueue, slack_value, *top);

							if (top->finish_time > top->period)
							{
								return NOT_SCHEDULABLE;
							}

							top->calcPORMS_PV(PV_PORMS, *timeline);
							top->next_start_time = top->period;
							top->period = top->period + top->deadline; 

							arrivalQueue->push(*top);
							highReadyQueue->erase(highReadyQueue->begin() + (highReadyQueue->size() - itr - 1));

							if (fabs(preemptTime - top->finish_time) < EPS)
							{
								preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
							}

							this->getTaskSetPORMSArea(*timeline, 1);

							if (top->exet < top->wcet) 
							{
								addNewSlack(slackQueue, *top);
							}

							PORMS_schedule_point_num++;
							this->flag = 0;
							return 0;
						}
						else
						{
#ifdef debug_print
							cout_trace << "SLACK6:::WE SHOULD NOT BE HERE\n\n" << endl;
#endif
						}
					}
				}
				else
				{
					itr++;
				}
			}
			if (slack_top->slack_value > 0.0 && this->flag)
			{
				if (slack_top->slack_value < (preemptTime - *timeline))
				{
					*timeline = *timeline + slack_top->slack_value;
					slack_top->slack_value = 0.0;
					slackQueue->pop();
				}
				else
				{
					slack_top->slack_value = slack_top->slack_value - (preemptTime - *timeline);
					*timeline = preemptTime;

					if (fabs(slack_top->slack_value - 0.0) < EPS)
					{
						slackQueue->pop();
					}
					preempt(highReadyQueue, lowReadyQueue, arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}
			}
			else
			{
				assert(slack_top->slack_value > 0.0);
			}
		}
	}
	return 0;
}

int CRTScheduling::runPORMS(PQV <CTasks, CPeriodComparator> highReadyQueue,
							 PQV <CTasks, CPromotionComparator> lowReadyQueue,
							 PQV <CTasks, CArrivalComparator> arrivalQueue, int lcm,
							 CTaskSetGenerator* generateTask, int withSlack)
{
	double timeline = 0.0;

	PQV <CSlack, CSlackComparator> slackQueue;

	while (timeline < lcm)
	{
		/*COND CODE: 111*/
		if ((!highReadyQueue.empty()) && (!lowReadyQueue.empty()) && (!arrivalQueue.empty()))
		{
			double preemptTime;
			int preemptType;		//1: arrival, 0: promotion

			CTasks currTask = highReadyQueue.top();
			CTasks promTask = lowReadyQueue.top();
			CTasks arrivTask = arrivalQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if (fabs(arrivTask.next_start_time - promTask.promotion_time) < EPS || arrivTask.next_start_time < promTask.promotion_time)
			{
				preemptTime = arrivTask.next_start_time;
				preemptType = 1;
			}
			else if (arrivTask.next_start_time > promTask.promotion_time)
			{
				preemptTime = promTask.promotion_time;
				preemptType = 0;
			}

			if (currTask.is_dummy == 1)
			{
				addDummy(&highReadyQueue, &arrivalQueue, &slackQueue, &currTask);
				continue;
			}

//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			if (!slackQueue.empty())
			{
				int schedulable = manageSlack(&highReadyQueue, &slackQueue, &lowReadyQueue, &arrivalQueue, preemptTime, preemptType, &timeline, lcm);
				if (schedulable == -1) {
					return NOT_SCHEDULABLE;
				}
			}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			else
			{
				currTask.start_time = timeline;
				if (currTask.is_preempted == 1)
				{
					currTask.finish_time = timeline + currTask.remaining_time;
					currTask.is_preempted = 0;
				}
				else
				{
					currTask.finish_time = timeline + currTask.exet;
				}

				// 1. Preempted: promo_time is in between s_time and f_time
				if ((preemptTime > currTask.start_time) && (preemptTime < currTask.finish_time))
				{
					currTask.remaining_time = currTask.finish_time - preemptTime;
					if (currTask.pv_start == 0)
					{
						currTask.original_start_time = currTask.start_time;
						currTask.pv_start = 1;
					}
					currTask.is_preempted = 1;
					double executed_time = preemptTime - currTask.start_time;
					timeline = timeline + executed_time;

					//printPreemptions(timeline, 1, currTask);

					if ((currTask.remaining_time > (lcm - timeline)) || (timeline > lcm) || (fabs(lcm - timeline) < EPS))
					{
						return NOT_SCHEDULABLE;
					}

					if (currTask.preference_level == 1) 
					{
						this->getTaskSetPORMSArea(timeline, 1);
					}
					else
					{
						this->getTaskSetPORMSArea(timeline, 0);
					}

					highReadyQueue.pop();
					highReadyQueue.push(currTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
				}

				// 2. Preempted: promo_time is equal to s_time
				else if (fabs(preemptTime - currTask.start_time) < EPS)
				{
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, preemptType);
					PORMS_schedule_point_num++;
					continue;
				}

				// 3. Preempted: promo_time is equal to f_time
				else if (fabs(preemptTime - currTask.finish_time) < EPS)
				{
					timeline = currTask.finish_time;
					//printPreemptions(timeline, 3, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					
					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif
					highReadyQueue.pop();
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, preemptType);

					/******** DYNAMIC CHECKING *******/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/*********************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}

				// no preemption, the task is done in this period.
				else if (preemptTime > currTask.finish_time)
				{
					timeline = currTask.finish_time;
					//printPreemptions(timeline, 0, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif
					highReadyQueue.pop();

					/******** DYNAMIC CHECKING *************/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/***************************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "PORMS:::1.WE SHOULD NOT BE HERE\n\ncurrTask.id: " << currTask.id << endl;
#endif
					/*tracefile << "=========== PORMS:::1 =========" << endl;
					for (int j = 0; j < ASAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->asap[j].wcet<<", period = "<<generateTask->asap[j].period \
							<<", id = "<<generateTask->asap[j].id<<", pref_level = "<<generateTask->asap[j].preference_level<<endl;
					}
					for (int j = 0; j < ALAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->alap[j].wcet<<", period = "<<generateTask->alap[j].period \
							<<", id = "<<generateTask->alap[j].id<<", pref_level = "<<generateTask->alap[j].preference_level<<endl;
					}*/
					return NOT_SCHEDULABLE;
				}
			}
		}

		/*COND CODE: 110*/
		else if ((!highReadyQueue.empty()) && (!lowReadyQueue.empty()) && arrivalQueue.empty())
		{
			CTasks currTask = highReadyQueue.top();
			CTasks promTask = lowReadyQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);
			
			if (currTask.is_dummy == 1)
			{
				addDummy(&highReadyQueue, &arrivalQueue, &slackQueue, &currTask);
				continue;
			}

//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			if (!slackQueue.empty())
			{
				int schedulable = manageSlack(&highReadyQueue, &slackQueue, &lowReadyQueue, &arrivalQueue, promTask.promotion_time, \
							0, &timeline, lcm);
				if (schedulable == -1) {
					return NOT_SCHEDULABLE;
				}
			}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			else
			{
				currTask.start_time = timeline;
				if (currTask.is_preempted == 1)
				{
					currTask.finish_time = timeline + currTask.remaining_time;
					currTask.is_preempted = 0;
				}
				else
				{
					currTask.finish_time = timeline + currTask.exet;
				}

				// check if the task can get preempted
				// 1. Preempted: promo_time is in between s_time and f_time
				if ((promTask.promotion_time > currTask.start_time) && (promTask.promotion_time < currTask.finish_time))
				{
					currTask.remaining_time = currTask.finish_time - promTask.promotion_time;
					if (currTask.pv_start == 0)
					{
						currTask.original_start_time = currTask.start_time;
						currTask.pv_start = 1;
					}
					currTask.is_preempted = 1;
					double executed_time = promTask.promotion_time - currTask.start_time;
					timeline = timeline + executed_time;

					//printPreemptions(timeline, 1, currTask);

					if (currTask.preference_level == 1) 
					{
						this->getTaskSetPORMSArea(timeline, 1);
					}
					else
					{
						this->getTaskSetPORMSArea(timeline, 0);
					}

					if ((currTask.remaining_time > (lcm - timeline)) || (timeline > lcm) || (fabs(lcm - timeline) < EPS))
					{
						return NOT_SCHEDULABLE;
					}

#ifdef debug_print
					cout_trace << "\ncurrTask.remaining_time: " << currTask.remaining_time << endl;
					cout_trace << "currTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
					//cout_trace << "6.PUSHING PROM TASK ID: " << promTask.id << " TO HIGH" << endl;
#endif
					highReadyQueue.pop();
					highReadyQueue.push(currTask);
					//lowReadyQueue.pop();
					//highReadyQueue.push(promTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 0);
					PORMS_schedule_point_num++;
				}

				// 2. Preempted: promo_time is equal to s_time
				else if (fabs(promTask.promotion_time - currTask.start_time) < EPS)
				{			
#ifdef debug_print
					cout_trace << "\nPREEMPTION #2\n" << endl;
					cout_trace << "\n----updating the promotion task states------\n" << endl;
					cout_trace << "\npromTask.period: " << promTask.period << endl;
					//cout_trace << "7.PUSHING PROM TASK ID: " << promTask.id << " TO HIGH" << endl;
#endif
					//lowReadyQueue.pop();
					//highReadyQueue.push(promTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 0);
					PORMS_schedule_point_num++;
					continue;
				}

				// 3. Preempted: promo_time is equal to f_time
				else if (fabs(promTask.promotion_time - currTask.finish_time) < EPS)
				{
					timeline = currTask.finish_time;

					//printPreemptions(timeline, 3, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;

					cout_trace << "\n----updating the promotion task states------\n" << endl;
					cout_trace << "\npromTask.period: " << promTask.period << endl;
					cout_trace << "8.PUSHING PROM TASK ID: " << promTask.id << " TO HIGH" << endl;

#endif
					highReadyQueue.pop();
					//lowReadyQueue.pop();
					//highReadyQueue.push(promTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 0);

					/******** DYNAMIC CHECKING ******************************************************************************/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/*********************************************************************************************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}

				// no preemption, the task is done in this period.
				else if (promTask.promotion_time > currTask.finish_time)
				{
					timeline = currTask.finish_time;

					//printPreemptions(timeline, 0, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif
					highReadyQueue.pop();

					/******** DYNAMIC CHECKING ******************************************************************************/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/*********************************************************************************************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "PORMS:::2.WE SHOULD NOT BE HERE\n\ncurrTask.id: " << currTask.id << endl;
#endif
					/*tracefile << "=========== PORMS:::2 =========" << endl;
					for (int j = 0; j < ASAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->asap[j].wcet<<", period = "<<generateTask->asap[j].period \
							<<", id = "<<generateTask->asap[j].id<<", pref_level = "<<generateTask->asap[j].preference_level<<endl;
					}
					for (int j = 0; j < ALAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->alap[j].wcet<<", period = "<<generateTask->alap[j].period \
							<<", id = "<<generateTask->alap[j].id<<", pref_level = "<<generateTask->alap[j].preference_level<<endl;
					}*/
					return NOT_SCHEDULABLE;
				}
			}
		}

		/*COND CODE: 011*/
		else if ((!lowReadyQueue.empty()) && (!arrivalQueue.empty()) && highReadyQueue.empty())
		{
			double preemptTime;
			int preemptType;

			CTasks promTask = lowReadyQueue.top();
			CTasks arrivTask = arrivalQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if (fabs(arrivTask.next_start_time - promTask.promotion_time) < EPS || arrivTask.next_start_time < promTask.promotion_time)
			{
				preemptTime = arrivTask.next_start_time;
				preemptType = 1;
			}
			else if (arrivTask.next_start_time > promTask.promotion_time)
			{
				preemptTime = promTask.promotion_time;
				preemptType = 0;
			}

			if ((timeline < preemptTime)) 
			{
				double idle_time = preemptTime - timeline;
#ifdef debug_print
				cout_trace << "\n----6.1. system gets " << idle_time <<" unit idle time------" << endl;
#endif				
				//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				if (!slackQueue.empty())
				{
					consumeSlack(&slackQueue, idle_time);
				}
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

				timeline = preemptTime;
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, preemptType);
				this->getTaskSetPORMSArea(timeline, 0);
				PORMS_schedule_point_num++;
				continue;
			}
			else if (fabs(timeline - preemptTime) < EPS)
			{
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, preemptType);
				PORMS_schedule_point_num++;
				continue;
			}
			else 
			{
				cout << "ERROR!" << endl;
				return NOT_SCHEDULABLE;
			}
		}

		/*COND CODE: 101*/
		else if((!highReadyQueue.empty()) && (!arrivalQueue.empty()) && lowReadyQueue.empty())
		{
			CTasks currTask = highReadyQueue.top();
			CTasks arrivTask = arrivalQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if (currTask.is_dummy == 1)
			{
				addDummy(&highReadyQueue, &arrivalQueue, &slackQueue, &currTask);
				continue;
			}

//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			if (!slackQueue.empty())
			{
				int schedulable = manageSlack(&highReadyQueue, &slackQueue, &lowReadyQueue, &arrivalQueue, arrivTask.next_start_time, \
							1, &timeline, lcm);
				if (schedulable == -1) {
					return NOT_SCHEDULABLE;
				}
			}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			else
			{
				currTask.start_time = timeline;
				if (currTask.is_preempted == 1)
				{
					currTask.finish_time = timeline + currTask.remaining_time;
					currTask.is_preempted = 0;
				}
				else
				{
					currTask.finish_time = timeline + currTask.exet;
				}

				// check if the task can get preempted
				// 1. Preempted: promo_time is in between s_time and f_time
				if ((arrivTask.next_start_time > currTask.start_time) && (arrivTask.next_start_time < currTask.finish_time))
				{
					currTask.remaining_time = currTask.finish_time - arrivTask.next_start_time;
					if (currTask.pv_start == 0)
					{
						currTask.original_start_time = currTask.start_time;
						currTask.pv_start = 1;
					}
					currTask.is_preempted = 1;
					double executed_time = arrivTask.next_start_time - currTask.start_time;
					timeline = timeline + executed_time;

					//printPreemptions(timeline, 1, currTask);

					if ((currTask.remaining_time > (lcm - timeline)) || (timeline > lcm) || (fabs(lcm - timeline) < EPS))
					{
						return NOT_SCHEDULABLE;
					}

					if (currTask.preference_level == 1) 
					{
						this->getTaskSetPORMSArea(timeline, 1);
					}
					else
					{
						this->getTaskSetPORMSArea(timeline, 0);
					}

#ifdef debug_print
					cout_trace << "\ncurrTask.remaining_time: " << currTask.remaining_time << endl;
					cout_trace << "currTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
					//cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
					highReadyQueue.pop();
					highReadyQueue.push(currTask);
					//arrivalQueue.pop();			
					//highReadyQueue.push(arrivTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 1);
					PORMS_schedule_point_num++;
				}

				// 2. Preempted: promo_time is equal to s_time
				else if (fabs(arrivTask.next_start_time - currTask.start_time) < EPS)
				{
#ifdef debug_print
					cout_trace << "\nPREEMPTION #2\n" << endl;
					cout_trace << "\n----updating the arriving task states------\n" << endl;
					cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
					//arrivalQueue.pop();			
					//highReadyQueue.push(arrivTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 1);
					PORMS_schedule_point_num++;
					continue;
				}

				// 3. Preempted: promo_time is equal to f_time
				else if (fabs(arrivTask.next_start_time - currTask.finish_time) < EPS)
				{
					timeline = currTask.finish_time;

					//printPreemptions(timeline, 3, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
					cout_trace << "\n----updating the arrival task states------\n" << endl;
					//cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
					highReadyQueue.pop();
					//arrivalQueue.pop();
					//highReadyQueue.push(arrivTask);
					preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 1);

					/******** DYNAMIC CHECKING ********/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/**********************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}

				// no preemption, the task is done in this period.
				else if (arrivTask.next_start_time > currTask.finish_time)
				{
					timeline = currTask.finish_time;

					//printPreemptions(timeline, 0, currTask);

					if (currTask.finish_time > currTask.period)
					{
						return NOT_SCHEDULABLE;
					}

					currTask.calcPORMS_PV(PV_PORMS, timeline);
					currTask.next_start_time = currTask.period;
					currTask.period = currTask.period + currTask.deadline;
					currTask.remaining_time = currTask.exet;

#ifdef debug_print
					//commonPrint(cout_trace, "\ncurrTask.next_start_time: " + currTask.next_start_time + endl);
					cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
					cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif
					highReadyQueue.pop();

					/******** DYNAMIC CHECKING *********/
					if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
					{
						addNewSlack(&slackQueue, currTask);
					}
					/***********************************/

					if (currTask.preference_level == 1) {
						this->getTaskSetPORMSArea(timeline, 1);
						arrivalQueue.push(currTask);
					}
					else {
						this->getTaskSetPORMSArea(timeline, 0);
						currTask.promotion_time = currTask.deadline + currTask.promotion_time;
						//lowReadyQueue.push(currTask);
						arrivalQueue.push(currTask);
					}
					PORMS_schedule_point_num++;
				}
				else
				{
#ifdef debug_print
					cout_trace << "PORMS:::4.WE SHOULD NOT BE HERE\n\ncurrTask.id: " << currTask.id << endl;
#endif
					/*tracefile << "=========== PORMS:::4 =========" << endl;
					for (int j = 0; j < ASAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->asap[j].wcet<<", period = "<<generateTask->asap[j].period \
							<<", id = "<<generateTask->asap[j].id<<", pref_level = "<<generateTask->asap[j].preference_level<<endl;
					}
					for (int j = 0; j < ALAP_TASK_COUNT; j++)
					{
						tracefile <<"Task "<< j <<", WCET = "<<generateTask->alap[j].wcet<<", period = "<<generateTask->alap[j].period \
							<<", id = "<<generateTask->alap[j].id<<", pref_level = "<<generateTask->alap[j].preference_level<<endl;
					}*/
					return NOT_SCHEDULABLE;
				}
			}
		}

		/*COND CODE: 100*/
		else if (lowReadyQueue.empty() && arrivalQueue.empty() && (!highReadyQueue.empty()))
		{
			CTasks currTask = highReadyQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if (currTask.is_dummy == 1)
			{
				addDummy(&highReadyQueue, &arrivalQueue, &slackQueue, &currTask);
				continue;
			}

//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			if (!slackQueue.empty())
			{
				int schedulable = manageSlack(&highReadyQueue, &slackQueue, &lowReadyQueue, &arrivalQueue, lcm + 1, \
							-1, &timeline, lcm);
				if (schedulable == -1) {
					return NOT_SCHEDULABLE;
				}
			}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			else
			{
				currTask.start_time = timeline;
				if (currTask.is_preempted == 1)
				{
					currTask.finish_time = timeline + currTask.remaining_time;
					currTask.is_preempted = 0;
				}
				else
				{
					currTask.finish_time = timeline + currTask.exet;
				}
				timeline = currTask.finish_time;

				//printPreemptions(timeline, 0, currTask);

				if (currTask.finish_time > currTask.period)
				{
					return NOT_SCHEDULABLE;
				}

				currTask.calcPORMS_PV(PV_PORMS, timeline);
				currTask.next_start_time = currTask.period;
				currTask.period = currTask.period + currTask.deadline;
				currTask.remaining_time = currTask.exet;

#ifdef debug_print
				cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
				cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif
				highReadyQueue.pop();

				/******** DYNAMIC CHECKING *********/
				if ((currTask.exet < currTask.wcet) && (withSlack == 1)) 
				{
					addNewSlack(&slackQueue, currTask);
				}
				/***********************************/

				if (currTask.preference_level == 1) {
					this->getTaskSetPORMSArea(timeline, 1);
					arrivalQueue.push(currTask);
				}
				else {
					this->getTaskSetPORMSArea(timeline, 0);
					currTask.promotion_time = currTask.deadline + currTask.promotion_time;
					//lowReadyQueue.push(currTask);
					arrivalQueue.push(currTask);
				}
				PORMS_schedule_point_num++;
			}
		}

		/*COND CODE: 001*/
		else if(highReadyQueue.empty() && lowReadyQueue.empty() && (!arrivalQueue.empty()))
		{
			CTasks arrivTask = arrivalQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if ((timeline < arrivTask.next_start_time)) 
			{
				double idle_time = arrivTask.next_start_time - timeline;
#ifdef debug_print
				cout_trace << "\n----5.1. system gets " << idle_time <<" idle time------\n" << endl;
#endif			
				//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@
				if (!slackQueue.empty())
				{	
					consumeSlack(&slackQueue, idle_time);					
				}
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

				//arrivalQueue.pop();
				//highReadyQueue.push(arrivTask);
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 1);
				timeline = arrivTask.next_start_time;
				this->getTaskSetPORMSArea(timeline, 0);
				PORMS_schedule_point_num++;
				continue;
			}
			else if (fabs(timeline - arrivTask.next_start_time) < EPS)
			{
				//arrivalQueue.pop();
				//highReadyQueue.push(arrivTask);
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 1);
				PORMS_schedule_point_num++;
				continue;
			}
			else 
			{
				cout << "ERROR!" << endl;
				return NOT_SCHEDULABLE;
			}	
		}

		/*COND CODE: 010*/
		else if(highReadyQueue.empty() && arrivalQueue.empty() && (!lowReadyQueue.empty()))
		{
			CTasks promTask = lowReadyQueue.top();

			//printTasks(timeline, highReadyQueue, lowReadyQueue, arrivalQueue);

			if ((timeline < promTask.promotion_time)) 
			{
					double idle_time = promTask.promotion_time - timeline;
#ifdef debug_print
				cout_trace << "\n----2.1. system gets " << (promTask.promotion_time - timeline) <<" idle time------\n" << endl;
#endif		
				//@@@@@@@@@@@@@@ Dynamic @@@@@@@@@@@@@@@@@@@@
				if (!slackQueue.empty())
				{
					consumeSlack(&slackQueue, idle_time);
				}
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

				//lowReadyQueue.pop();
				//highReadyQueue.push(promTask);
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 0);
				timeline = promTask.promotion_time;
				this->getTaskSetPORMSArea(timeline, 0);
				PORMS_schedule_point_num++;
				continue;
			}
			else if (fabs(timeline - promTask.promotion_time) < EPS)
			{
				//lowReadyQueue.pop();
				//highReadyQueue.push(promTask);
				preempt(&highReadyQueue, &lowReadyQueue, &arrivalQueue, 0);
				PORMS_schedule_point_num++;
				continue;
			}
			else 
			{
				cout << "ERROR!" << endl;
				return NOT_SCHEDULABLE;
			}
		}

		/*COND CODE: 000*/
		else if(highReadyQueue.empty() && lowReadyQueue.empty() && arrivalQueue.empty())
		{
			cout << "THIS WAS NOT EXPECTED!" << endl;
			exit(0);
		}
	}

	//tracefile.close();
	return SCHEDULABLE;
}

int CRTScheduling::runRMS(PQV <CTasks, CPeriodComparator> singleReadyQueue, 
						   PQV < CTasks, CArrivalComparator > arrivalQueue, int  lcm,
						   CTaskSetGenerator* generateTask)
{
	double timeline = 0.0;

	//ofstream tracefile;
	//tracefile.open ("SHLDNT_B_HERE.rms.trace");

	while (timeline < lcm)
	{
		if ((!singleReadyQueue.empty()) && (!arrivalQueue.empty())) 
		{
			CTasks currTask = singleReadyQueue.top();
			CTasks arrivTask = arrivalQueue.top();

#ifdef debug_print_rms
			cout_trace << "\n|| RMS TIMELINE: " << timeline << " ||\n" << endl;
			cout_trace << "currTask.id: " << currTask.id << endl;
			cout_trace << "currTask.required_time: " << currTask.remaining_time << endl;
			cout_trace << "currTask.period: " << currTask.period << endl;

			cout_trace << "arrivTask.id: " << arrivTask.id << endl;
			cout_trace << "arrivTask.next_start_time: " << arrivTask.next_start_time << endl;
			//cout_trace << "arrivTask.promotion_time: " << arrivTask.promotion_time << endl;
			cout_trace << "arrivTask.required_time: " << arrivTask.remaining_time << endl;
			cout_trace << "arrivTask.deadline: " << arrivTask.period << endl;
#endif
			currTask.start_time = timeline;
			if (currTask.is_preempted == 1)
			{
				currTask.finish_time = timeline + currTask.remaining_time;

				currTask.is_preempted = 0;
			}
			else
			{
				currTask.finish_time = timeline + currTask.wcet;
			}

			// check if the task can get preempted
			// 1. Preempted: promo_time is in between s_time and f_time
			if ((arrivTask.next_start_time > currTask.start_time) && (arrivTask.next_start_time < currTask.finish_time))
			{

#ifdef debug_print_rms
				cout_trace << "\nPREEMPTION #1\n" << endl;
				cout_trace << "executed task ID: " << currTask.id << endl;
				cout_trace << "start time: " << currTask.start_time << endl;
				cout_trace << "  executing......" << endl;
				cout_trace << "finish time: " << currTask.finish_time << endl;
#endif
				currTask.remaining_time = currTask.finish_time - arrivTask.next_start_time;
				if (currTask.pv_start == 0)
				{
					//cout_trace << "COMES HERE" << endl;
					currTask.original_start_time = currTask.start_time;
					currTask.pv_start = 1;
				}
				currTask.is_preempted = 1;
				double executed_time = arrivTask.next_start_time - currTask.start_time;
				timeline = timeline + executed_time;

				if ((currTask.remaining_time > (lcm - timeline)) || (timeline > lcm) || (fabs(lcm - timeline) < EPS))
				{
					return NOT_SCHEDULABLE;
				}

				if (currTask.preference_level == 1) 
				{
					this->getTaskSetRMSArea(timeline, 1);
				}
				else
				{
					this->getTaskSetRMSArea(timeline, 0);
				}

#ifdef debug_print_rms
				cout_trace << "\ncurrTask.remaining_time: " << currTask.remaining_time << endl;
				cout_trace << "currTask.next_start_time: " << currTask.next_start_time << endl;
				cout_trace << "currTask.next_deadline: " << currTask.period << endl;
				cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
				singleReadyQueue.pop();
				arrivalQueue.pop();

				singleReadyQueue.push(currTask);
				singleReadyQueue.push(arrivTask);

				RMS_schedule_point_num++;
			}

			// 2. Preempted: promo_time is equal to s_time
			else if (fabs(arrivTask.next_start_time - currTask.start_time) < EPS)
			{		

#ifdef debug_print_rms
				cout_trace << "\nPREEMPTION #2\n" << endl;
				cout_trace << "\n----updating the arriving task states------\n" << endl;
				//cout_trace << "\npromTask.period: " << promTask.period << endl;
#endif
				arrivalQueue.pop();

#ifdef debug_print_rms
				cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
#endif
				singleReadyQueue.push(arrivTask);
				RMS_schedule_point_num++;
				continue;
			}
			// 3. Preempted: promo_time is equal to f_time
			else if (fabs(arrivTask.next_start_time - currTask.finish_time) < EPS)
			{
#ifdef debug_print_rms
				cout_trace << "\nPREEMPTION #3\n" << endl;
				cout_trace << "executed task ID: " << currTask.id << endl;
				cout_trace << "start time: " << currTask.start_time << endl;
				cout_trace << "  executing......" << endl;
				cout_trace << "finish time: " << currTask.finish_time << endl;
#endif

				//timeline = timeline + currTask.remaining_time;
				timeline = currTask.finish_time;

				if (currTask.finish_time > currTask.period)
				{
					return NOT_SCHEDULABLE;
				}

				if (currTask.preference_level == 1) 
				{
					this->getTaskSetRMSArea(timeline, 1);
				}
				else
				{
					this->getTaskSetRMSArea(timeline, 0);
				}

				currTask.calcRMS_PV(PV_RMS);

				currTask.next_start_time = currTask.period;
				currTask.period = currTask.period + currTask.deadline; 

#ifdef debug_print_rms
				cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
				cout_trace << "currTask.next_deadline: " << currTask.period << endl;
				cout_trace << "\n----updating the arrival task states------\n" << endl;
				cout_trace << "PUSHING ARRIV TASK ID: " << arrivTask.id << " TO HIGH" << endl;
				//cout_trace << "\npromTask.period: " << promTask.period << endl;
#endif

				singleReadyQueue.pop();
				arrivalQueue.pop();

				// TODO:: push this currTask to arrivQueue.
				//highReadyQueue.push(currTask);
				arrivalQueue.push(currTask);
				singleReadyQueue.push(arrivTask);

				RMS_schedule_point_num++;
			}

			// otherwise:
			// no preemption, the task is done in this period.
			// 1. update period for the next tern
			// 2. pop
			// 3. push in highReadyQueue again
			else if (arrivTask.next_start_time > currTask.finish_time)
			{

#ifdef debug_print_rms
				cout_trace << "\nNO PREEMPTION\n" << endl;
				cout_trace << "executed task ID: " << currTask.id << endl;
				cout_trace << "start time: " << currTask.start_time << endl;
				cout_trace << "  executing......" << endl;
				cout_trace << "finish time: " << currTask.finish_time << endl;
#endif

				//timeline = timeline + currTask.remaining_time;
				timeline = currTask.finish_time;

				if (currTask.finish_time > currTask.period)
				{
					return NOT_SCHEDULABLE;
				}

				if (currTask.preference_level == 1) 
				{
					this->getTaskSetRMSArea(timeline, 1);
				}
				else
				{
					this->getTaskSetRMSArea(timeline, 0);
				}

				currTask.calcRMS_PV(PV_RMS);

				currTask.next_start_time = currTask.period;
				currTask.period = currTask.period + currTask.deadline; 

#ifdef debug_print_rms
				cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
				cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif

				singleReadyQueue.pop();
				// TODO:: push this currTask to arrivQueue.
				//highReadyQueue.push(currTask);
				arrivalQueue.push(currTask);

				RMS_schedule_point_num++;
			}
			else
			{
#ifdef debug_print_rms
				cout_trace << "RMS:::WE SHOULD NOT BE HERE\n\ncurrTask.id: " << currTask.id << endl;
#endif

				/*tracefile << "=========== RMS::: =========" << endl;
				for (int j = 0; j < ASAP_TASK_COUNT; j++)
				{
					tracefile <<"Task "<< j <<", WCET = "<<generateTask->asap[j].wcet<<", period = "<<generateTask->asap[j].period \
						<<", id = "<<generateTask->asap[j].id<<", pref_level = "<<generateTask->asap[j].preference_level<<endl;
				}
				for (int j = 0; j < ALAP_TASK_COUNT; j++)
				{
					tracefile <<"Task "<< j <<", WCET = "<<generateTask->alap[j].wcet<<", period = "<<generateTask->alap[j].period \
						<<", id = "<<generateTask->alap[j].id<<", pref_level = "<<generateTask->alap[j].preference_level<<endl;
				}*/
				return NOT_SCHEDULABLE;
			}
		}
		else if (arrivalQueue.empty())
		{
			CTasks currTask = singleReadyQueue.top();

#ifdef debug_print_rms
			cout_trace << "\n|| RMS TIMELINE: " << timeline << " ||\n" << endl;
			cout_trace << "currTask.id: " << currTask.id << endl;
			cout_trace << "currTask.required_time: " << currTask.remaining_time << endl;
			cout_trace << "currTask.period: " << currTask.period << endl;
#endif

			currTask.start_time = timeline;
			if (currTask.is_preempted == 1)
			{
				currTask.finish_time = timeline + currTask.remaining_time;
				currTask.is_preempted = 0;
			}
			else
			{
				currTask.finish_time = timeline + currTask.wcet;
			}

#ifdef debug_print_rms
			cout_trace << "\nNO PREEMPTION HERE\n" << endl;
			cout_trace << "executed task ID: " << currTask.id << endl;
			cout_trace << "start time: " << currTask.start_time << endl;
			cout_trace << "  executing......" << endl;
			cout_trace << "finish time: " << currTask.finish_time << endl;
#endif

			//timeline = timeline + currTask.remaining_time;
			timeline = currTask.finish_time;

			if (currTask.preference_level == 1) 
			{
				this->getTaskSetRMSArea(timeline, 1);
			}
			else
			{
				this->getTaskSetRMSArea(timeline, 0);
			}

			currTask.calcRMS_PV(PV_RMS);

			currTask.next_start_time = currTask.period;
			currTask.period = currTask.period + currTask.deadline; 

#ifdef debug_print_rms
			cout_trace << "\ncurrTask.next_start_time: " << currTask.next_start_time << endl;
			cout_trace << "currTask.next_deadline: " << currTask.period << endl;
#endif

			singleReadyQueue.pop();
			arrivalQueue.push(currTask);

			RMS_schedule_point_num++;		
		}
		else if (singleReadyQueue.empty())
		{
			CTasks arrivTask = arrivalQueue.top();

#ifdef debug_print_rms
			cout_trace << "\n|| RMS TIMELINE: " << timeline << " ||\n" << endl;
			cout_trace << "arrivTask.id: " << arrivTask.id << endl;
			cout_trace << "arrivTask.next_start_time: " << arrivTask.next_start_time << endl;
			//cout_trace << "arrivTask.promotion_time: " << arrivTask.promotion_time << endl;
			cout_trace << "arrivTask.required_time: " << arrivTask.remaining_time << endl;
			cout_trace << "arrivTask.deadline: " << arrivTask.period << endl;
#endif

			if ((timeline < arrivTask.next_start_time)) 
			{

#ifdef debug_print_rms
				cout_trace << "\n----system gets " << (arrivTask.next_start_time - timeline) <<" idle time------\n" << endl;
#endif
				timeline = arrivTask.next_start_time;
				this->getTaskSetRMSArea(timeline, 0);
			}
			arrivalQueue.pop();
			singleReadyQueue.push(arrivTask);
			RMS_schedule_point_num++;
		}
	}
	//tracefile.close();
	return SCHEDULABLE;
}


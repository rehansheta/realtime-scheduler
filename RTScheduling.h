#include <iostream>
#include <queue>
#include <cstdlib>

#include "QComparators.h"
#include "Constants.h"
#include "TaskSetGenerator.h"
#include "Tasks.h"
#include "Utility.h"

using namespace std;

class CRTScheduling
{

public:

	double LCM_for_area;
	int flag;
	CTaskSetGenerator taskSet;
	int PORMS_schedule_point_num;		//num of scheduling points for PORMS
	int RMS_schedule_point_num;		//num of scheduling points for RMS

	static double *PV_PORMS;  		//preference value for each task in PORMS
	static double *PV_RMS;  		//preference value for each task in RMS

	static double porms_taskset_area_point;
	static double porms_taskset_area;
	static double rms_taskset_area_point;
	static double rms_taskset_area;


	CRTScheduling();
	~CRTScheduling();
	void createPORMSQueues();
	void createRMSQueues();
	int runPORMS(PQV<CTasks, CPeriodComparator> highReadyQueue, 
				  PQV<CTasks, CPromotionComparator> lowReadyQueue,
				  PQV<CTasks, CArrivalComparator> arrivalQueue, int lcm,
				  CTaskSetGenerator* generateTask, int withSlack);
	int manageSlack(PQV <CTasks, CPeriodComparator> *highReadyQueue,
					 PQV <CSlack, CSlackComparator> *slackQueue,
					 PQV <CTasks, CPromotionComparator> *lowReadyQueue,
					 PQV <CTasks, CArrivalComparator> *arrivalQueue, 
					 double preemptTime, int preemptType, double* timeline, int lcm);
	void consumeSlack(PQV <CSlack, CSlackComparator> *slackQueue, double idle_time);
	void addNewSlack(PQV <CSlack, CSlackComparator> *slackQueue, CTasks top);
	void pushForwardSlack(PQV <CSlack, CSlackComparator> *slackQueue, double slack_value, CTasks top);
	void addDummy(PQV <CTasks, CPeriodComparator> *highReadyQueue,
					PQV <CTasks, CArrivalComparator> *arrivalQueue,
					PQV <CSlack, CSlackComparator> *slackQueue, CTasks* currTask);
	void preempt(PQV<CTasks, CPeriodComparator> *highReadyQueue, 
					PQV<CTasks, CPromotionComparator> *lowReadyQueue,
					PQV<CTasks, CArrivalComparator> *arrivalQueue, int preemptType);

	int runRMS(PQV<CTasks, CPeriodComparator> singleReadyQueue, 
				PQV < CTasks, CArrivalComparator > arrivalQueue, int lcm,
				CTaskSetGenerator* generateTask);

	void printTasks(double timeline, PQV<CTasks, CPeriodComparator> highReadyQueue, 
					PQV<CTasks, CPromotionComparator> lowReadyQueue,
					PQV<CTasks, CArrivalComparator> arrivalQueue);
	void printPreemptions(double timeline, int preemption_type, CTasks currTask);
	//void printTraces(double timeline, int type, String traceID, CTasks top, double slack);

	double getTaskSetPORMSArea(double finish_time, int task_level);
	double getTaskSetRMSArea(double finish_time, int task_level);

};

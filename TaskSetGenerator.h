#ifndef TASK_SET_GENERATOR
#define TASK_SET_GENERATOR


#include <iostream>
#include <queue>

#include "Tasks.h"
#include "Constants.h"

using namespace std;

class CTaskSetGenerator
{

public:

	int LCM;
	double* task_wcet;
	double* task_exet;
	double* task_periods;
	double* task_util;

	// for PO-RMS:  
	static CTasks* asap;
	static CTasks* alap;
	static CTasks* alap_dummy;

	// for RMS: static
	static CTasks* tasks;

	CTaskSetGenerator();
	~CTaskSetGenerator();
	//void generatePORMSTaskSet();
	double GeneratePORMSTaskSet(double dyn_load);
	void GeneratePORMSTaskSet_dummy(double dyn_load, double min_period);
	void UUniFast(int TaskNum, double Utot, int start_id);
	void generateRMSTaskSet();
	int getTaskSetLCM();
	void printAllTasks(int i); // for PORMS + RMS

};
#endif
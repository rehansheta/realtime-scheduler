#ifndef TASKS
#define TASKS


#include <iostream>
#include <queue>

using namespace std;

class CTasks
{
	public:
		int id;
		int u;
		int promoting;
		int is_dummy;				// 1: task is a dummy; 0: task is original
		int is_preempted;			// required to update the finish time
		int pv_start;				// required to update the pv value
		double wcet;
		double exet;				// execution time
		double period;				//? this periods is updated after each round.
		double start_time;			// would depend on the promotion time.
		double original_start_time;		// marks the original start time of a preempted task.
		double next_start_time;			// required to stop multiple execution for the same round of tasks.
		double finish_time;			// finish time. should be updated when preempted.
		double required_time;			// unnecessary
		double deadline;			// this remains unchanged. used it as the fixed period.
		double remaining_time;		
		double promotion_time;
		double original_promotion_time;
		int preference_level;			// 0: low, 1: high
	
	CTasks();
	CTasks(double period, double wcet, double exet, int id, int pref_level);	
	~CTasks();
	void getPromotionTime(CTasks* alap, CTasks* asap);
	void getPromotionTime_dummy(CTasks* alap, CTasks* asap_dummy);
	void calcPORMS_PV(double* PV_PORMS, double timeline);
	void calcRMS_PV(double* PV_RMS);

};

#endif

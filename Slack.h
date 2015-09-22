#ifndef SLACK
#define SLACK


#include <iostream>
#include <queue>

using namespace std;

class CSlack
{
	public:
		int slack_id;
		int slack_is_preempted;			// required to update the finish time
		double slack_value;
		double slack_wcet;
		double slack_exet;				// execution time
		double slack_period;			//? this periods is updated after each round.
		double slack_deadline;			// this remains unchanged. used it as the fixed period.
		int slack_preference_level;	// 0: low, 1: high
	
	CSlack();	
	~CSlack();

};

#endif

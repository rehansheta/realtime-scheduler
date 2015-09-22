// constructing priority queues

#include <iostream>
#include <queue>
#include <math.h>

#include "QComparators.h"
#include "Constants.h"

using namespace std;


CPeriodComparator::CPeriodComparator()
{

}

CPeriodComparator::~CPeriodComparator()
{

}


bool CPeriodComparator::operator() (CTasks lhs, CTasks rhs) 
{
	//return rhs.period < lhs.period; // how does it work? details?
	if (rhs.deadline < lhs.deadline) return true;
	if (fabs(rhs.deadline - lhs.deadline) < EPS && rhs.id < lhs.id) return true;
	return false;
}


CSlackComparator::CSlackComparator()
{

}

CSlackComparator::~CSlackComparator()
{

}


bool CSlackComparator::operator() (CSlack lhs, CSlack rhs) 
{
	//return rhs.period < lhs.period; // how does it work? details?
	if (rhs.slack_deadline < lhs.slack_deadline) return true;
	if (fabs(rhs.slack_deadline - lhs.slack_deadline) < EPS && rhs.slack_id < lhs.slack_id) return true;
	return false;
}


CPromotionComparator::CPromotionComparator()
{

}

CPromotionComparator::~CPromotionComparator()
{

}

bool CPromotionComparator::operator() (CTasks lhs, CTasks rhs) //const
{
	/*if (rhs.promotion_time < lhs.promotion_time) return true;
	if (fabs(rhs.promotion_time - lhs.promotion_time) < EPS && rhs.id < lhs.id) return true;
	return false;*/
	if (rhs.promotion_time < lhs.promotion_time) return true;
	if (fabs(rhs.promotion_time - lhs.promotion_time) < EPS && rhs.id < lhs.id) return true;
	return false;
}

CArrivalComparator::CArrivalComparator()
{

}

CArrivalComparator::~CArrivalComparator()
{

}

bool CArrivalComparator::operator() (CTasks lhs, CTasks rhs) 
{
	if (rhs.next_start_time < lhs.next_start_time) return true;
	if ((fabs(rhs.next_start_time - lhs.next_start_time) < EPS) && (rhs.id < lhs.id)) return true;
	return false;
}


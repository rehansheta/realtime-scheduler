// constructing priority queues
#ifndef QCOMPARATORS
#define QCOMPARATORS


#include <iostream>
#include <queue>


#include "Tasks.h"
#include "Slack.h"

using namespace std;

class CPeriodComparator
{

public:
	CPeriodComparator();
	~CPeriodComparator();
	bool operator() (CTasks lhs, CTasks rhs);
};

class CSlackComparator
{

public:
	CSlackComparator();
	~CSlackComparator();
	bool operator() (CSlack lhs, CSlack rhs);
};

class CPromotionComparator
{

public:
	CPromotionComparator();
	~CPromotionComparator();
	bool operator() (CTasks lhs, CTasks rhs);
};

class CArrivalComparator
{

public:
	CArrivalComparator();
	~CArrivalComparator();
	bool operator() (CTasks lhs, CTasks rhs);
};

#endif
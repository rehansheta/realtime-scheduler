#ifndef UTILITY
#define UTILITY

#include <iostream>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <queue>
#include <stdarg.h>
#include <sstream>
#include <stdio.h>
#ifdef PORMS_WINDOWS
#include <windows.h>
#endif
#ifndef PORMS_WINDOWS
#include <sys/time.h>
#include <unistd.h>
#endif

#include <string.h>

using namespace std;

template<class T, class Compare>
class PQV : public vector<T> {
public:
	Compare comp;

	PQV(Compare cmp = Compare()) : comp(cmp) {
	}
	
	vector<T>* getVector() {
		return this;
	}

	void push(const T& x) {
		this->push_back(x);
		typename std::vector<T>::iterator secondLast = this->end();
		secondLast--;
		std::inplace_merge(this->begin(), secondLast, this->end(), comp);
	}

	void pop() {
		this->pop_back();
	}

	T top() {
		return this->back();
	}

	void update_queue() {
	}
};

//void writeHeaders(ofstream& genPV_noDummy, ofstream& genPV_Dummy, ofstream& genPV_noSlack, ofstream& genPV_Slack, ofstream& tracePV_noDummy, 
//				 ofstream& tracePV_Dummy, ofstream& tracePV_noSlack, ofstream& tracePV_Slack)
//{
//	time_t t = time(0);   // get time now
//    struct tm * now = localtime( & t );
//	stringstream ss;
//    ss << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' <<  now->tm_mday << endl;
//	
//	genPV_noDummy << "NO dummy NO slack" << endl;
//	genPV_noDummy << ss.str() << endl;
//	genPV_noDummy << "Dynamic Load:: fixed (50%)" << endl;
//	genPV_noDummy << "Total Task Count:: varies (50 - 100)" << endl;
//	genPV_noDummy << "System Utilization: fixed (50%)" << endl;
//
//	tracePV_noDummy << "NO dummy NO slack" << endl;
//	tracePV_noDummy << ss.str() << endl;
//	tracePV_noDu#include <iostream>

//	tracePV_noDummy << "Total Task Count:: varies (50 - 100)" << endl;
//	tracePV_noDummy << "System Utilization: fixed (50%)" << endl;
//
//	genPV_Dummy << "WITH dummy NO slack" << endl;
//	genPV_Dummy << ss.str() << endl;
//	genPV_Dummy << "Dynamic Load:: fixed (50%)" << endl;
//	genPV_Dummy << "Total Task Count:: varies (50 - 100)" << endl;
//	genPV_Dummy << "System Utilization: fixed (50%)" << endl;
//
//	tracePV_Dummy << "WITH dummy NO slack" << endl;
//	tracePV_Dummy << ss.str() << endl;
//	tracePV_Dummy << "Dynamic Load:: fixed (50%)" << endl;
//	tracePV_Dummy << "Total Task Count:: varies (50 - 100)" << endl;
//	tracePV_Dummy << "System Utilization: fixed (50%)" << endl;
//
//	genPV_noSlack << "NO dummy NO slack" << endl;
//	genPV_noSlack << ss.str() << endl;
//	genPV_noSlack << "Dynamic Load:: fixed (50%)" << endl;
//	genPV_noSlack << "Total Task Count:: varies (50 - 100)" << endl;
//	genPV_noSlack << "System Utilization: fixed (50%)" << endl;
//
//	tracePV_noSlack << "NO dummy NO slack" << endl;
//	tracePV_noSlack << ss.str() << endl;
//	tracePV_noSlack << "Dynamic Load:: fixed (50%)" << endl;
//	tracePV_noSlack << "Total Task Count:: varies (50 - 100)" << endl;
//	tracePV_noSlack << "System Utilization: fixed (50%)" << endl;
//
//	genPV_Slack << "NO dummy WITH slack" << endl;
//	genPV_Slack << ss.str() << endl;
//	genPV_Slack << "Dynamic Load:: fixed (50%)" << endl;
//	genPV_Slack << "Total Task Count:: varies (50 - 100)" << endl;
//	genPV_Slack << "System Utilization: fixed (50%)" << endl;
//
//	tracePV_Slack << "NO dummy WITH slack" << endl;
//	tracePV_Slack << ss.str() << endl;
//	tracePV_Slack << "Dynamic Load:: fixed (50%)" << endl;
//	tracePV_Slack << "Total Task Count:: varies (50 - 100)" << endl;
//	tracePV_Slack << "System Utilization: fixed (50%)" << endl;
//}

int getGCD ( int a, int b );
int getLCM(int a, int b);
long long GetTimeMs64();
void srand32(unsigned x);
long rand32();
float prob();
int myrandom( int i1, int i2 ) ;

#endif

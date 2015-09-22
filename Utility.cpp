#include "Utility.h"

/*prob - return random from Uniform[ 0, 1 )*/
static  long    randx = 1;
static  double divis = 2147483648.0;  /*  = 2**31, cycle of rand()  */


int getGCD ( int a, int b )
{
    int c;
    while ( a != 0 )
    {
        c = a;
        a = b%a;
        b = c;
    }
    return b;
}


int getLCM(int a, int b)
{
    return (a * b) / getGCD(a, b);
}


long long GetTimeMs64()
{
	unsigned long long ret;
#ifdef PORMS_WINDOWS
	FILETIME ft;cout << "TIME T :: " << (float)t << endl;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	//ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */
	ret *= 100;
#endif

#ifndef PORMS_WINDOWS
	struct timeval start, end;
	gettimeofday(&start, NULL);
	ret = ((start.tv_sec) * 1000000 + start.tv_usec) * 10;
#endif
	return ret;
}


void srand32(unsigned x)
{ 
	randx = x; 
}


long rand32()
{
  return((randx = randx * 1103515245 + 12345) & 0x7fffffff);
}


float prob()        
{
	double y;
	y = rand32();
	return((float)(y/divis));
}


/* random - return integer random from Equiprob( i1, i2 )*/
int myrandom( int i1, int i2 ) 
{
    return( i1 + (int)((i2-i1+1)*prob()) ) ;    /* chopping is used */
}

/* required for all trace output prints */
template <class T>
char* to_string(T* x)
{
 char *tmp = reinterpret_cast<char*>(x);
 if (strlen(tmp) == 0) tmp = "0";
 return tmp;
}

void debugFilePrint(int n_args, ...)
{
#ifdef debug_print
 va_list ap;
 va_start(ap, n_args);
    
 for (int i = 0; i < n_args; i++) {
  char* a = va_arg(ap, char *);
  cout << a << " ";
 }
 cout << endl;
 va_end(ap);
#endif
}

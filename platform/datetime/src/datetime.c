#include <datetime.h>
#include <osal.h>
#include <coov3.h>
#include <common.h>
#ifdef OSX
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

COO_DCL(DateTime,ull,getMicroTime);
COO_DEF_RET_NOARGS(DateTime,ull,getMicroTime) {
	return this->getNanoTime()/1000000;
}
COO_DCL(DateTime,uint,getSecondTime);
COO_DEF_RET_NOARGS(DateTime,uint,getSecondTime) {
	return this->getNanoTime()/1000000000L;
}


COO_DCL(DateTime,ull,getNanoTime);
COO_DEF_RET_NOARGS(DateTime,ull,getNanoTime) {
#ifdef OSX
	return mach_absolute_time();
#else
	// clock_gettime requires -lrt
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
#endif
}


DateTime DateTime_New(OE oe) {
	DateTime res = oe->getmem(sizeof(*res));
	COO_ATTACH(DateTime,res,getNanoTime);
	COO_ATTACH(DateTime,res,getMicroTime);
	COO_ATTACH(DateTime,res,getSecondTime);
	return res;
}

void Register_DatetimeDefaultConstructor(OE oe) {
	oe->provideSystemLibrary(DATE_TIME_LIBRARY,(DefaultConstructor)DateTime_New);
}

void DateTime_Destroy(DateTime * instance) {

}

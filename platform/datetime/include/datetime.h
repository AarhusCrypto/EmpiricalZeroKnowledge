#ifndef DATETIME_H
#define DATETIME_H
#include <osal.h>

typedef struct _datetime_ {
	/**
	 * Time in seconds since EPOCH (1/1/1970).
	 */
	uint (*getSecondTime)();

	/**
	 * Time in Nanoseconds since EPOCH (1/1/1970).
	 */
	ull (*getNanoTime)();

	/**
	 * Time in Microseconds since EPOCH (1/1/1970).
	 */
	ull (*getMicroTime)();

} * DateTime;

DateTime DateTime_New(OE oe);
void DateTime_Destroy(DateTime * instance);
void Register_DatetimeDefaultConstructor(OE oe);

#endif

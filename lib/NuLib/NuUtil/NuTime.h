
#include "NuCommon.h"

#ifndef _NUTIME_H
#define _NUTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Date */
void NuGetToday(char *Today);	    /* YYYYMMDD */
void NuGetToday2(char *Today);	    /* YYYY-MM-DD */
int NuGetTodayWDay();               /* WeekDay 0:SUN 1:MON .. */

/* Time */
void NuGetTime(char *Time);         /* hh:mm:ss.SSSSSS */
void NuGetTime2(char *Time);        /* hh:mm:ss */
void NuGetTime_HHMMSS(char *Time);  /* hhmmss */

void NuTimeToStr(struct timeval *tv, char *Time);         /* hh:mm:ss.SSSSSS */
/* tools */
time_t NuHMSToTime(const char *Hms);

typedef struct _NuDateTime_t NuDateTime_t;

int NuDateTimeNew(NuDateTime_t **pDateTime, bool IsLocal);
void NuDateTimeFree(NuDateTime_t *pDateTime);

void NuDateTimeInit(NuDateTime_t *pDateTime, int MoveHour, bool IsLocal);

int NuDateTimeSet(NuDateTime_t *pDateTime);
void NuDateTimeSet3(NuDateTime_t *pDateTime, int year, int month, int day, int hour, int min, int sec, int ms, int us);
#define NuDateTimeSet2(pDateTime, year, month, day, hour, min, sec)	NuDateTimeSet3((pDateTime), (year), (month), (day), (hour), (min), (sec), 0, 0)

typedef enum _DateTime_FMT
{
	DATETIME_FMT1 = 0,      /* YYYY-MM-DD hh:mm:ss */
	DATETIME_FMT2,          /* YYYY-MM-DD hh:mm:ss.SSS */
	DATETIME_FMT3,          /* YYYY-MM-DD hh:mm:ss.SSSSSS */
	DATETIME_FMT4,          /* YYYYMMDD-hh:mm:ss */
	DATETIME_FMT5,          /* YYYYMMDD-hh:mm:ss.SSS */
	DATETIME_FMT6,          /* YYYYMMDD-hh:mm:ss.SSSSSS */
} DateTime_FMT;

void NuDateTimeGet(NuDateTime_t *pDateTime, char *pRtnStr, DateTime_FMT Format);
#define NuDateTimeGet1(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT1 )
#define NuDateTimeGet2(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT2 )
#define NuDateTimeGet3(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT3 )
#define NuDateTimeGet4(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT4 )
#define NuDateTimeGet5(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT5 )
#define NuDateTimeGet6(NuDTime, pRtnTime)   NuDateTimeGet( (NuDTime), (pRtnTime), DATETIME_FMT6 )

#ifdef __cplusplus
}
#endif

#endif /* _NUTIME_H */


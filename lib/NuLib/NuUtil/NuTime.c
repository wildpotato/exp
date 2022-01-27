
#include "NuCommon.h"
#include "NuCStr.h"
#include "NuTime.h"

#define NuGmTime(ptm, t)    gmtime_r((t), ptm)
#define NuLocalTime(ptm, t) localtime_r((t), ptm)
#define NuDateTimeHR	3600    /* 1 hr  = 60 * 60 = 3600 sec      */
#define NuDateTimeDay	86400   /* 1 day = 60 * 60 *24 = 86400 sec */

#define	SecInHour(hr)	(hr)*3600

/* Date */
/* ====================================================================== */
void NuGetToday(char *Today)
{
    time_t      now = time(NULL);
    struct tm   ltimer;
    struct tm   *ptm = localtime_r(&now, &ltimer);

    /* YYYYMMDD */
    NuCStrPrintInt(Today, ptm->tm_year + 1900, 4);
    NuCStrPrintInt(Today + 4, ptm->tm_mon + 1, 2);
    NuCStrPrintInt(Today + 6, ptm->tm_mday, 2);

    return;
}

void NuGetToday2(char *Today)
{
    time_t      now = time(NULL);
    struct tm   ltimer;
    struct tm   *ptm = localtime_r(&now, &ltimer);

    /* YYYY-MM-DD */
    NuCStrPrintInt(Today, ptm->tm_year + 1900, 4);
    *(Today + 4) = '-';
    NuCStrPrintInt(Today + 5, ptm->tm_mon + 1, 2);
    *(Today + 7) = '-';
    NuCStrPrintInt(Today + 8, ptm->tm_mday, 2);

    return;
}

int NuGetTodayWDay()
{
    time_t      now = time(NULL);
    struct tm   ltimer;
    struct tm   *ptm = localtime_r(&now, &ltimer);

    return ptm->tm_wday;
}

/* Date */
/* ====================================================================== */
void NuGetTime(char *Time)
{
    time_t          now, tmp;
    struct timeval  tv;

    gettimeofday(&tv, NULL);

    now = (tv.tv_sec + SecInHour(8)) % NuDateTimeDay;

    /* hh:mm:ss.SSSSSS */
    NuLongCopyToLPadZero(Time, tmp = now / 3600, 2);
    *(Time + 2) = ':';

    now -= tmp * 3600;
    NuLongCopyToLPadZero(Time + 3, tmp = now / 60, 2);
    *(Time + 5) = ':';
    NuLongCopyToLPadZero(Time + 6, now - tmp * 60, 2);
    *(Time + 8) = '.';
    NuLongToStrLPadZero(Time + 9, tv.tv_usec, 6);

    return;
}

void NuGetTime2(char *Time)
{
    time_t  now = time(NULL), tmp;

    now = (now + SecInHour(8)) % NuDateTimeDay;

    /* hh:mm:ss */
    NuLongCopyToLPadZero(Time, tmp = now / 3600, 2);
    *(Time + 2) = ':';

    now -= tmp * 3600;
    NuLongCopyToLPadZero(Time + 3, tmp = now / 60, 2);
    *(Time + 5) = ':';
    NuLongToStrLPadZero(Time + 6, now - tmp * 60, 2);

    return;
}

void NuGetTime_HHMMSS(char *Time)
{
    time_t          now, tmp;
    struct timeval  tv;

    gettimeofday(&tv, NULL);

    now = (tv.tv_sec + SecInHour(8)) % NuDateTimeDay;

    /* hhmmss */
    NuCStrPrintLong(Time, tmp = now / 3600, 2);

    now -= tmp * 3600;
    NuCStrPrintLong(Time + 2, tmp = now / 60, 2);
    NuCStrPrintLong(Time + 4, now - tmp * 60, 2);

    return;
}

time_t NuHMSToTime(const char *Hms)
{
    time_t      now = time(NULL);
    struct tm   ltimer;
    struct tm   *ptm = localtime_r(&now, &ltimer);

    /* HH:MM:SS */
    if(strlen(Hms) < 8)
    {
        return 0;
    }

    ptm->tm_hour = ((*Hms - '0') * 10) + *(Hms + 1) - '0';
    ptm->tm_min = ((*(Hms + 3) - '0') * 10) + *(Hms + 4) - '0';
    ptm->tm_sec = ((*(Hms + 6) - '0') * 10) + *(Hms + 7) - '0';

    return mktime(ptm);
}

void NuTimeToStr(struct timeval *tv, char *Time)
{
    time_t          now, tmp;

    now = (tv->tv_sec + SecInHour(8)) % NuDateTimeDay;

    /* hh:mm:ss.SSSSSS */
    NuLongCopyToLPadZero(Time, tmp = now / 3600, 2);
    *(Time + 2) = ':';

    now -= tmp * 3600;
    NuLongCopyToLPadZero(Time + 3, tmp = now / 60, 2);
    *(Time + 5) = ':';
    NuLongCopyToLPadZero(Time + 6, now - tmp * 60, 2);
    *(Time + 8) = '.';
    NuLongToStrLPadZero(Time + 9, tv->tv_usec, 6);

    return;
}


/* base function */
/* ====================================================================== */

struct _NuDateTime_t
{
    struct tm       Tm;
	struct timeval  TimeVal;
    int             MoveHour;
	bool            IsLocal;
};

int NuDateTimeNew(NuDateTime_t **pDateTime, bool IsLocal)
{
    int RC = NU_OK;

    if(!((*pDateTime) = (NuDateTime_t *)malloc(sizeof(NuDateTime_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    NuDateTimeInit((*pDateTime), 0, IsLocal);

EXIT:
    if(RC < 0)
    {
        NuDateTimeFree(*pDateTime);
    }

    return RC;
}

void NuDateTimeFree(NuDateTime_t *pDateTime)
{
    if(pDateTime != NULL)
    {
        free(pDateTime);
    }

    return;
}

void NuDateTimeInit(NuDateTime_t *pDateTime, int MoveHour, bool IsLocal)
{
    memset(pDateTime, 0, sizeof(NuDateTime_t));
    pDateTime->MoveHour = MoveHour;
    pDateTime->IsLocal = IsLocal;
    return;
}

int NuDateTimeSet(NuDateTime_t *pDateTime)
{
    time_t  now = time(NULL) + (pDateTime->MoveHour * NuDateTimeHR);

    if(pDateTime->IsLocal)
    {
        NuLocalTime(&(pDateTime->Tm), &now);
    }
    else
    {
        NuGmTime(&(pDateTime->Tm), &now);
    }

    if(gettimeofday(&(pDateTime->TimeVal), NULL) < 0)
    {
        return NU_FAIL;
    }

    return NU_OK;
}

void NuDateTimeSet3(NuDateTime_t *pDateTime, int year, int month, int day, int hour, int min, int sec, int ms, int us)
{
    struct tm   tm_tmp;
    time_t      now;

    tm_tmp.tm_year = year - 1900;
    tm_tmp.tm_mon  = month - 1;
    tm_tmp.tm_mday = day;
    tm_tmp.tm_hour = hour;
    tm_tmp.tm_min  = min;
    tm_tmp.tm_sec  = sec;

    now = mktime(&tm_tmp) + (pDateTime->MoveHour * NuDateTimeHR);

    if(pDateTime->IsLocal)
    {
        NuLocalTime(&(pDateTime->Tm), &now);
    }
    else
    {
        NuGmTime(&(pDateTime->Tm), &now);
    }

    pDateTime->TimeVal.tv_usec = ms * 1000 + us;

    return;
}

/* YYYY-MM-DD HH:MM:SS */
static void _PrintYMDHMS(struct tm *pTm, char *Str)
{
    NuIntCopyToLPadZero(Str, pTm->tm_year + 1900, 4);
    *(Str + 4) = '-';
    NuIntCopyToLPadZero(Str + 5, pTm->tm_mon + 1, 2);
    *(Str + 7) = '-';
    NuIntCopyToLPadZero(Str + 8, pTm->tm_mday, 2);
    *(Str + 10) = ' ';
    NuIntCopyToLPadZero(Str + 11, pTm->tm_hour, 2);
    *(Str + 13) = ':';
    NuIntCopyToLPadZero(Str + 14, pTm->tm_min, 2);
    *(Str + 16) = ':';
    NuIntToStrLPadZero(Str + 17, pTm->tm_sec, 2);

    return;
}

/* YYYYMMDD-hh:mm:ss */
static void _PrintYMDHMS2(struct tm *pTm, char *Str)
{
    NuIntCopyToLPadZero(Str, pTm->tm_year + 1900, 4);
    NuIntCopyToLPadZero(Str + 4, pTm->tm_mon + 1, 2);
    NuIntCopyToLPadZero(Str + 6, pTm->tm_mday, 2);
    *(Str + 8) = '-';
    NuIntCopyToLPadZero(Str + 9, pTm->tm_hour, 2);
    *(Str + 11) = ':';
    NuIntCopyToLPadZero(Str + 12, pTm->tm_min, 2);
    *(Str + 14) = ':';
    NuIntToStrLPadZero(Str + 15, pTm->tm_sec, 2);

    return;
}

static void _PrintDotMs(struct timeval *TimeVal, char *Str)
{
    *Str = '.';
    NuIntToStrLPadZero(Str + 1, TimeVal->tv_usec / 1000, 3);

    return;
}

static void _PrintDotUs(struct timeval *TimeVal, char *Str)
{
    *Str = '.';
    NuIntToStrLPadZero(Str + 1, TimeVal->tv_usec, 6);

    return;
}

void NuDateTimeGet(NuDateTime_t *pDateTime, char *pRtnStr, DateTime_FMT Format)
{
    struct tm   *ptm = &(pDateTime->Tm);
    
    switch(Format)
    {
    case DATETIME_FMT6:
        /* YYYYMMDD-hh:mm:ss.SSSSSS */
        _PrintYMDHMS2(ptm, pRtnStr);
        _PrintDotUs(&(pDateTime->TimeVal), pRtnStr + 17);

        break;
    case DATETIME_FMT5:
        /* YYYYMMDD-hh:mm:ss.SSS */
        _PrintYMDHMS2(ptm, pRtnStr);
        _PrintDotMs(&(pDateTime->TimeVal), pRtnStr + 17);

        break;
    case DATETIME_FMT4:
        /* YYYYMMDD-hh:mm:ss */
        _PrintYMDHMS2(ptm, pRtnStr);

        break;
    case DATETIME_FMT3:
        /* YYYY-MM-DD hh:mm:ss.SSSSSS */
        _PrintYMDHMS(ptm, pRtnStr);
        _PrintDotUs(&(pDateTime->TimeVal), pRtnStr + 19);
    
        break;
    case DATETIME_FMT2:
        /* YYYY-MM-DD hh:mm:ss.SSS */
        _PrintYMDHMS(ptm, pRtnStr);
        _PrintDotMs(&(pDateTime->TimeVal), pRtnStr + 19);

        break;
    case DATETIME_FMT1:
    default:
        /* YYYY-MM-DD hh:mm:ss */
        _PrintYMDHMS(ptm, pRtnStr);

        break;
    }

    return;
}


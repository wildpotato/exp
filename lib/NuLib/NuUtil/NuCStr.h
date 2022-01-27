#ifndef _NUCSTR_H
#define _NUCSTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "NuCommon.h"


void NuCStrReplaceRangeChr(char *CStr, char Chr, char NewChr, size_t Len);
void NuCStrReplaceChr(char *CStr, char Chr, char NewChr);
void NuCStrRTrimChr(char *CStr, char Chr);
void NuCStrLTrimChr(char *CStr, char Chr);
void NuCStrSubStr(char *SubStr, const char *Source, size_t Len);
int NuCStrPrintInt(char *CStr, int Value, int Length);
int NuCStrPrintLong(char *CStr, long Value, int Length);
int NuCStrToInt(const char *CStr, size_t Len);
long NuCStrToLong(const char *CStr, size_t Len);
float NuCStrToFloat(const char *CStr, size_t Len);
double NuCStrToDouble(const char *CStr, size_t Len);

bool NuCStrIsNumeric(const char *CStr);

typedef void (*NuCStrSplitFn)(unsigned int idx, const char *Msg, size_t MsgSize, void *Argu);
void NuCStrSplit(const char *CStr, char Sep, size_t Len, NuCStrSplitFn Fn, void *Argu);

void NuHexDump (void *addr, int len);

/* NuHexDumpStr already obsolete. please use NuHexDumpToString */
void NuHexDumpStr (void *addr, int len, char *output, int output_sz);
void NuHexDumpToString (void *addr, int len, char *output, int output_sz);
#define HexDumpNeedSize(len) ((NU_RALIGN((len), 16) * 5) + 8 + 2 + 1)

char *NuCharToBinString(char c, char *output);

/* tools for int/uint/long/ulong to string 
 * it will padding '0' in string  left to assign length
 * */
int NuIntCopyToLPadZero(char *str, int val, int padLen);
int NuUIntCopyToLPadZero(char *str, unsigned int val, int padLen);
int NuLongCopyToLPadZero(char *str, long val, int padLen);
int NuULongCopyToLPadZero(char *str, unsigned long val, int padLen);

static inline int NuIntToStrLPadZero(char *str, int val, int padLen) {
    str[padLen] = '\0';
    return NuIntCopyToLPadZero(str, val, padLen);
}

static inline int NuUIntToStrLPadZero(char *str, unsigned int val, int padLen) {
    str[padLen] = '\0';
    return NuUIntCopyToLPadZero(str, val, padLen);
}

static inline int NuLongToStrLPadZero(char *str, long val, int padLen) {
    str[padLen] = '\0';
    return NuLongCopyToLPadZero(str, val, padLen);
}

static inline int NuULongToStrLPadZero(char *str, unsigned long val, int padLen) {
    str[padLen] = '\0';
    return NuULongCopyToLPadZero(str, val, padLen);
}


#ifdef __cplusplus
}
#endif

#endif /* _NUSCTR_H */



#include "NuCommon.h"

#ifndef _NUNUM_H
#define _NUNUM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuNum_t NuNum_t;

int NuNumNew(NuNum_t **pNum, int Digit);
void NuNumFree(NuNum_t *pNum);
void NuNumFromStr(NuNum_t *pNum, char *NumStr);

void NuNumReset(NuNum_t *pNum);
int NuNumToStr(NuNum_t *pNum, char *NumStr);

/* pResult = pNum1 + pNum2*/
void NuNumAdd(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult);

/* pResult = pNum1 - pNum2 */
void NuNumSubstract(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult);

/* pResult = pNum1 * pNum2 */
void NuNumMultiply(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult);

/* pResult = pNum1 / pNum2, provided pNum2 is bounded in unsigned integer up to fractional. */
void NuNumDivide(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult, int AccuDigit);

/* Return: Positive - pNum1 > pNum2, Zero - pNum1 == pNum2, Negative - pNum1 < pNum2 */
int NuNumCompare(NuNum_t *pNum1, NuNum_t *pNum2);

/* pNum = pNum * 10^(Digit) */
void NuNumLeftShift(NuNum_t *pNum, int Digit);

/* pNum = pNum / 10^(Digit) */
void NuNumRightShift(NuNum_t *pNum, int Digit);

#ifdef __cplusplus
}
#endif

#endif /* _NUNUM_H */


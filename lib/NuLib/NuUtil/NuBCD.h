
#ifndef _NUBCD_H
#define _NUBCD_H

#ifdef __cplusplus
extern "C" {
#endif

extern int NuBCDMapping[256];

#define NuBCD2Dec(BCD)  NuBCDMapping[BCD]


#ifdef __cplusplus
}
#endif

#endif /* _NUBCD_H */


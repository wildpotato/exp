#ifndef _NUFIXMSG_H
#define _NUFIXMSG_H

#ifdef __cplusplus
extern "C" {
#endif
#include "NuCommon.h"
#include "NuBuffer.h"
#include "NuUtil.h"
#include "NuCStr.h"
#include "NuStr.h"
#include "NuHash.h"

#define NuFixMsgVerLen                      8       /* FIX.4.3 */
#define NuFixMsgDelimiter                   '\001'
#define NuFixMsgTerminator                  '\0'
#define NuFixMsgTagValSep                   '='
#define NuFixMsgDefaultVer                  "\0"

#define NuFixMsgKindInit                    0x00
#define NuFixMsgKindHeader                  0x01
#define NuFixMsgKindBody                    0x02
#define NuFixMsgKindRepeatingGroupLeader    0x04
#define NuFixMsgKindRepeatingGroupMember    0x08
#define NuFixMsgKindShell                   0x10

#define NuFixMsgDefaultTagStrLen            6
#define NuFixMsgDefaultValueLen             10
#define NuFixMsgDefaultTagNum               100

typedef struct _NuFixTag_t                  NuFixTag_t;
typedef struct _NuFixMsg_t                  NuFixMsg_t;

/* NuFixMsg_t size */
extern size_t NuFixMsgSz;

/* NuFixMsgNew include the NuFixMsgInit, 
 * NufixMsgFree include the NuFixMsgDestroy.
 * if you want to control the memory by yourself, call Init & Destroy is good 
 * */
int NuFixMsgInit(NuFixMsg_t *pFixMsg);
void NuFixMsgDestroy(NuFixMsg_t *pFixMsg);
/* Construtor/Terminator. */
int NuFixMsgNew(NuFixMsg_t **pFixMsg);
void NuFixMsgFree(NuFixMsg_t *pFixMsg);

void NuFixMsgClear(NuFixMsg_t *pFixMsg);

/* Template design tool. */
int NuFixMsgTemplateAddTag(NuFixMsg_t *pFixMsg, int Tag, size_t ExpectLen, int BelongGroup, int Kind);
#define NuFixMsgTemplateAddHeader(pFixMsg, Tag, ExpectLen)                      NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), 0, NuFixMsgKindHeader)

#define NuFixMsgTemplateAddBody(pFixMsg, Tag, ExpectLen)                        NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), 0, NuFixMsgKindBody)

#define NuFixMsgTemplateAddHeaderGroupLeader(pFixMsg, Tag, ExpectLen)           NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), 0, NuFixMsgKindHeader|NuFixMsgKindRepeatingGroupLeader)

#define NuFixMsgTemplateAddHeaderGroupMember(pFixMsg, Leading, Tag, ExpectLen)  NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), (Leading), NuFixMsgKindHeader|NuFixMsgKindRepeatingGroupMember)

#define NuFixMsgTemplateAddBodyGroupLeader(pFixMsg, Tag, ExpectLen)             NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), 0, NuFixMsgKindBody|NuFixMsgKindRepeatingGroupLeader)

#define NuFixMsgTemplateAddBodyGroupMember(pFixMsg, Leading, Tag, ExpectLen)    NuFixMsgTemplateAddTag((pFixMsg), (Tag), (ExpectLen), (Leading), NuFixMsgKindBody|NuFixMsgKindRepeatingGroupMember)

int NuFixMsgTemplateAddRepeatingGroup(NuFixMsg_t *pFixMsg, int Kind, int LeadingTag, int NodeNo, ...);

void NuFixMsgTemplateSort(NuFixMsg_t *pFixMsg);

/* Field operations. */

int NuFixMsgFieldAssign(NuFixMsg_t *pFixMsg, int Tag, char *pVal, size_t Len);
int NuFixMsgFieldSet(NuFixMsg_t *pFixMsg, int Tag, char *pVal, size_t Len);
int NuFixMsgFieldVPrintf(NuFixMsg_t *pFixMsg, int Tag, char *Format, va_list ArguList);
int NuFixMsgFieldPrintf(NuFixMsg_t *pFixMsg, int Tag, char *Format, ...);

int NuFixMsgFieldRemove(NuFixMsg_t *pFixMsg, int Tag);

/* Group operations. */
int NuFixMsgGroupAssign(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *pVal, size_t Len);
int NuFixMsgGroupSet(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *pVal, size_t Len);
int NuFixMsgGroupVPrintf(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *Format, va_list ArguList);
int NuFixMsgGroupPrintf(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *Format, ...);

int NuFixMsgGroupRemove(NuFixMsg_t *pFixMsg, int Tag, int Idx);

int NuFixMsgInstanceRemove(NuFixMsg_t *pFixMsg, int LeadingTag, int Idx);

/* Get Value from specified tag. */
NuFixTag_t *NuFixMsgFieldGet(NuFixMsg_t *pFixMsg, int Tag);
NuFixTag_t *NuFixMsgGroupGet(NuFixMsg_t *pFixMsg, int Tag, int Idx);
char *NuFixMsgGetVal(NuFixTag_t *pTag);
size_t NuFixMsgGetSize(NuFixTag_t *pTag);
int NuFixMsgGetKind(NuFixTag_t *pTag);

#define NuFixMsgGetFieldStr(pFixMsg, Tag)       NuFixMsgGetVal(NuFixMsgFieldGet((pFixMsg), (Tag)))
#define NuFixMsgGetGroupStr(pFixMsg, Tag, Idx)  NuFixMsgGetVal(NuFixMsgGroupGet((pFixMsg), (Tag), (Idx)))

#define NuFixMsgGetValChr(pFixTag, Idx) *(NuFixMsgGetVal((pFixTag)) + (Idx))
#define NuFixTagIsNULL(pFixTag)         ( NuFixMsgGetVal((pFixTag)) == NULL )

/* Generate the output message. */
#define NuFixMsgGenHB(pFixMsg)          NuFixMsgGen((pFixMsg), NuFixMsgKindHeader|NuFixMsgKindBody)

#define NuFixMsgGenBody(pFixMsg)        NuFixMsgGen((pFixMsg), NuFixMsgKindBody)

#define NuFixMsgGenMsg(pFixMsg)         NuFixMsgGen((pFixMsg), NuFixMsgKindShell|NuFixMsgKindHeader|NuFixMsgKindBody)

/* Load and parse the message. */
void NuFixMsgParse(NuFixMsg_t *pFixMsg, char *Msg);
void NuFixMsgParseByLen(NuFixMsg_t *pFixMsg, char *Msg, size_t MsgLen);

/* Generate the output message. */
NuStr_t *NuFixMsgGen(NuFixMsg_t *pFixMsg, int Gen);

char *NuFixMsgTakeOutMsg(NuFixMsg_t *pFixMsg);
size_t NuFixMsgGetOutMsgSize(NuFixMsg_t *pFixMsg);

unsigned int NuFixMsgGenCheckSum(const char *Msg, size_t Len);
void NuFixMsgTagExchange(NuFixMsg_t *pFixMsg, int Tag1, int Tag2);

#ifdef __cplusplus
}
#endif

#endif /* _NUFIXMSG_H */


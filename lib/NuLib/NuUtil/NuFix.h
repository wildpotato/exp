
#include "NuCommon.h"
#include "NuStr.h"

#ifndef _NUFIX_H
#define _NUFIX_H

#ifdef __cplusplus
extern "C" {
#endif

/* NuFixParser */
typedef struct _NuFixParser_t NuFixParser_t;
typedef struct _NuFixParserNode_t NuFixParserNode_t;

int NuFixParserNew(NuFixParser_t **Parser);
void NuFixParserFree(NuFixParser_t *Parser);

typedef bool (*NuFixParserFn)(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure);
NuFixParserNode_t *NuFixParserGetNode(NuFixParser_t *Parser, int Tag);
void NuFixParserSetCallback(NuFixParserNode_t *Node, NuFixParserFn Fn, void *Argu);
void NuFixParserRemoveCallback(NuFixParserNode_t *Node);
void NuFixParserSetToInt(NuFixParserNode_t *Node, int *Storage);
void NuFixParserSetToLong(NuFixParserNode_t *Node, long *Storage);
void NuFixParserSetToDouble(NuFixParserNode_t *Node, double *Storage);
void NuFixParserSetToFloat(NuFixParserNode_t *Node, float *Storage);
void NuFixParserSetToStr(NuFixParserNode_t *Node, NuStr_t *Storage);
bool NuFixParserForEach(NuFixParser_t *Parser, const char *Msg, void *Closure);
bool NuFixParserForEachByLen(NuFixParser_t *Parser, const char *Msg, int Len, void *Closure);
bool NuFixParserForEachByCB(NuFixParser_t *Parser, const char *Msg, int Len, NuFixParserFn DefaultFn, void *Closure);

/* fetch all tag-value to callback, it will not trigger Node callback */
bool NuFixParserFetchByCB(NuFixParser_t *Parser, const char *Msg, int Len, NuFixParserFn Fn, void *Closure);

/* NuFixComposer */
typedef struct _NuFixComposer_t NuFixComposer_t;
typedef struct _NuFixComposerField_t NuFixComposerField_t;
typedef struct _NuFixComposerRepeatingGroup_t NuFixComposerRepeatingGroup_t;

int NuFixComposerNew(NuFixComposer_t **Composer);
void NuFixComposerFree(NuFixComposer_t *Composer);
void NuFixComposerClear(NuFixComposer_t *Composer);

NuFixComposerField_t *NuFixComposerFindField(NuFixComposer_t *Composer, const char *Tag);
NuFixComposerField_t *NuFixComposerGetField(NuFixComposer_t *Composer, const char *Tag);
void NuFixComposerFieldSet(NuFixComposerField_t *Field, const char *Value, size_t ValueLen);
void NuFixComposerFieldSetUse(NuFixComposerField_t *Field, bool Use);
bool NuFixComposerFieldIsUse(NuFixComposerField_t *Field);

NuFixComposerRepeatingGroup_t *NuFixComposerGetRepeatingGroup(NuFixComposer_t *Composer, const char *Tag);
NuFixComposerField_t *NuFixComposerRepeatingGroupAddField(NuFixComposerRepeatingGroup_t *Group, const char *Tag);
NuFixComposerRepeatingGroup_t *NuFixComposerRepeatingGroupAddRepeatingGroup(NuFixComposerRepeatingGroup_t *Group, const char *Tag);
void NuFixComposerRepeatingGroupSetNo(NuFixComposerRepeatingGroup_t *Group, int No);
void NuFixComposerRepeatingGroupClear(NuFixComposerRepeatingGroup_t *Group);

const char *NuFixComposerCompose(NuFixComposer_t *Composer);

const char *NuFixComposerGetStr(NuFixComposer_t *Composer);
size_t NuFixComposerGetStrSize(NuFixComposer_t *Composer);


#ifdef __cplusplus
}
#endif

#endif /* _NUFIX_H */



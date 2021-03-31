#include "FixReader.hpp"

ez::FixTagValue ez::GetFixValue(const char *msg, int tag, bool firstTag)
{
    char tag2Search[16];
    int tagSize = snprintf(tag2Search, 16, firstTag ? "%d=" : "\001%d=", tag);
    
    // find tag position
    const char *findPos = strstr(msg, tag2Search);
    if (tagSize < 0 || findPos == nullptr)
        return {};
    
    // find value begin
    findPos += tagSize;
    
    // find value end
    const char *valueEndPos = strchr(findPos, '\001');
    
    if (valueEndPos == nullptr)
        return {};
    else
        return {findPos, valueEndPos};
}

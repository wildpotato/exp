
#include "NuCStr.h"

#define DecimalTable    "9876543210123456789"
#define HEXVAL(x)       (((x) >= 'A') ? ((x)-'A'+10) : (x) - '0')

void NuCStrReplaceRangeChr(char *CStr, char Chr, char NewChr, size_t Len)
{
    char    *d = CStr + Len;

    while(CStr != d)
    {
        if(*CStr == Chr)
        {
            *CStr = NewChr;
        }

        ++ CStr;
    }

    return;
}

void NuCStrReplaceChr(char *CStr, char Chr, char NewChr)
{
    while(*CStr != '\0')
    {
        if(*CStr == Chr)
        {
            *CStr = NewChr;
        }

        ++ CStr;
    }

    return;
}

void NuCStrRTrimChr(char *CStr, char Chr)
{
    char *p = NULL;
    int  len = strlen(CStr) - 1;

    if (len >= 0)
    {
        p = CStr + len;

        while(*p == Chr && p > CStr)
        {
            -- p;
        }

        *(++ p) = '\0';
    }

    return;
}

void NuCStrLTrimChr(char *CStr, char Chr)
{
    char *e = CStr + strlen(CStr);
    char *s = CStr;

    while(*s == Chr && s < e)
    {
        ++ s;
    }

    memmove(CStr, s, e - s + 1);

    return;
}

void NuCStrSubStr(char *SubStr, const char *Source, size_t Len)
{
    memcpy(SubStr, Source, Len);
    SubStr[Len] = '\0';

    return;
}

int NuCStrPrintInt(char *CStr, int Value, int Length)
{
    char    *ptr = CStr, *ptr1 = CStr;
    int     tmp_value = 0, len = 0;

    *ptr = '\0';

    do
    {
        tmp_value = Value;
        Value /= 10;
        *ptr ++ = DecimalTable[9 + (tmp_value - Value * 10)];
    }
    while(Value);

    if(tmp_value < 0)
    {
        *ptr ++ = '-';
    }

    len = ptr - ptr1;
    *ptr -- = '\0';

    while(ptr1 < ptr)
    {
        char tmp_char = *ptr;
        *ptr -- = *ptr1;
        *ptr1 ++ = tmp_char;
    }

    if(len < Length)
    {
        if(*CStr == '-')
        {
            ++ CStr;
			memmove(CStr + Length - len, CStr, len);
		}
		else
		{
			memmove(CStr + Length - len, CStr, len + 1);
		}

        memset(CStr, '0', Length - len);
    
        return Length;
    }

    return len;
}

int NuCStrPrintLong(char *CStr, long Value, int Length)
{
    char    *ptr = CStr, *ptr1 = CStr;
    int     len = 0;
    long    tmp_value = 0;

    *ptr = '\0';

    do
    {
        tmp_value = Value;
        Value /= 10;
        *ptr ++ = DecimalTable[9 + (tmp_value - Value * 10)];
    }
    while(Value);

    if(tmp_value < 0)
    {
        *ptr ++ = '-';
    }

    len = ptr - ptr1;
    *ptr -- = '\0';

    while(ptr1 < ptr)
    {
        char tmp_char = *ptr;
        *ptr -- = *ptr1;
        *ptr1 ++ = tmp_char;
    }

    if(len < Length)
    {
        if(*CStr == '-')
        {
            ++ CStr;
			memmove(CStr + Length - len, CStr, len);
		}
		else
		{
			memmove(CStr + Length - len, CStr, len + 1);
		}

        memset(CStr, '0', Length - len);
    
        return Length;
    }

    return len;
}

int NuCStrToInt(const char *CStr, size_t Len)
{
    int         Ret = 0, Sign = 1;
    const char  *End = CStr + Len;

	if (*CStr == '-')
	{
		Sign = -1;
		++ CStr;
	}

    do
    {
		if (*CStr == ',')
		{
			continue;
		}
        Ret = Ret * 10 + *CStr - '0';
    }
    while(++ CStr < End);

    return (Ret * Sign);
}

long NuCStrToLong(const char *CStr, size_t Len)
{
    long        Ret = 0, Sign = 1;
    const char  *End = CStr + Len;

	if (*CStr == '-')
	{
		Sign = -1;
		++ CStr;
	}

    do
    {
		if (*CStr == ',')
		{
			continue;
		}
        Ret = Ret * 10 + *CStr - '0';
    }
    while(++ CStr < End);

    return (Ret * Sign);
}

float NuCStrToFloat(const char *CStr, size_t Len)
{
    float       Ret = 0, Factor = 1, Sign  = 1;
    const char  *End = CStr + Len;

	if (*CStr == '-')
	{
		Sign = -1;
		++ CStr;
	}

    while(CStr < End && *CStr != '.')
    {
		if (*CStr == ',')
		{
			++CStr;
			continue;
		}
        Ret = Ret * 10 + *CStr - '0';
        ++ CStr;
    }
	
	if(CStr < End)
	{
		while(++ CStr < End)
		{
			if (*CStr == ',')
			{
				continue;
			}
			Factor *= 0.1;
			Ret += (float)(*CStr - '0') * Factor;
		}
	}

    return (Ret * Sign);
}

double NuCStrToDouble(const char *CStr, size_t Len)
{
    double      Ret = 0, Factor = 1, Sign = 1;
    const char  *End = CStr + Len;

	if (*CStr == '-')
	{
		Sign = -1;
		++ CStr;
	}

    while(CStr < End && *CStr != '.')
	{
		if (*CStr == ',')
		{
			++ CStr;
			continue;
		}
		Ret = Ret * 10 + *CStr - '0';
		++ CStr;
    }

	if (CStr < End)
	{
		while(++ CStr < End)
		{
			if (*CStr == ',')
			{
				continue;
			}
			Factor *= 0.1;
			Ret += (double)(*CStr - '0') * Factor;
		}
	}

    return (Ret * Sign);
}

bool NuCStrIsNumeric(const char *CStr)
{
	int dot = 0;

	if(*CStr == '+' || *CStr == '-')
	{
		++ CStr;
	}

	do
	{
		if(!isdigit(*CStr))
		{
			if (*CStr == '.' && dot == 0)
			{
				dot++;
			}
			else if (*CStr != ',')
			{
				return false;
			}
		}

		++ CStr;
	}
	while(*CStr != '\0');

	return true;
}

void NuCStrSplit(const char *CStr, char Sep, size_t Len, NuCStrSplitFn Fn, void *Argu)
{
    const char  *pSep = NULL;
	const char  *End = CStr + Len;
	unsigned int idx = 0;

	for (;;)
    {
		pSep = strchr(CStr, Sep);
		if (pSep != NULL && pSep < End)
		{
			Fn(idx, CStr, pSep - CStr, Argu);
			CStr = pSep + 1;
		}
		else
		{
			Fn(idx, CStr, End - CStr, Argu);
			break;
		}
		++idx;
    }

    return;
}

void NuHexDump (void *addr, int len) 
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf ("  %s\n", buff);

			// Output the offset.
			printf ("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf (" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf ("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf ("  %s\n", buff);
}

void NuHexDumpStr (void *addr, int len, char *output, int output_sz)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;
    char *p = (char *)output;

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0){
				p += sprintf (p, "  %s\n", buff);
            }

			// Output the offset.
            p += sprintf (p, "  %04x ", i);
		}

		// Now the hex code for the specific character.
		p += sprintf(p, " %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
        } else {
			buff[i % 16] = pc[i];
        }
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		p += sprintf(p, "   ");
		i++;
	}

	// And print the final ASCII bit.
	p += sprintf(p, "  %s", buff);
}

void NuHexDumpToString (void *addr, int len, char *output, int output_sz) 
{
	int i;
	char buff[16] = {0};
	char *ptr = output;
	char *end = output + output_sz;
	char *pc = (char *)addr;

	// Process every byte in the data.
	for (i = 0; i < len; i++) 
	{
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) 
		{
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
			{
				if (ptr + strlen(buff) < end)
				{
					ptr += sprintf(ptr, "  %s\n", buff);
				}
				else
				{
					goto EXIT;
				}
			}

			// Output the offset.
			if (ptr + 7 < end)
			{
				ptr += sprintf(ptr, "  %04x ", i);
			}
			else
			{
				goto EXIT;
			}
		}

		// Now the hex code for the specific character.
		if (ptr + 4 < end)
		{
			ptr += sprintf(ptr, " %02x", pc[i]);
		}
		else
		{
			goto EXIT;
		}

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) 
	{
		if (ptr + 4 < end)
		{
			ptr += sprintf (ptr, "   ");
		}
		else
		{
			goto EXIT;
		}
		i++;
	}

	// And print the final ASCII bit.
	if (ptr + strlen(buff) < end)
	{
		ptr += sprintf(ptr, "  %s\n", buff);
	}
	else
	{
		goto EXIT;
	}
	*ptr = '\0';
EXIT:
	*end = '\0';
}

char *NuCharToBinString(char c, char *output)
{
	int len = 7;
	char *ptr = output;

	while(len >= 0)
	{
		char mask = 1 << len;
		*ptr = !!(mask & c) + '0';
		len--;

		ptr++;
	}
	*ptr = '\0';

	return output;
}

/* fast number to string convert */
#define NumTable    "0123456789"
#define NumCopyToStr(T, S, V, L) do { \
    unsigned int _idx = L; \
    T _val = V; \
    do{ \
        V = _val/10; \
        S[_idx - 1]=NumTable[(_val - V *10)]; \
        _val = V; \
        --_idx; \
    } while(_idx>0); \
} while(0)

int NuUIntCopyToLPadZero(char *str, unsigned int val, int padLen)
{
    NumCopyToStr(unsigned int, str, val, padLen);
    return padLen;
}

int NuIntCopyToLPadZero(char *str, int val, int padLen)
{
    unsigned int uval;
    if (val >= 0) 
    {
        uval = (unsigned int)val;
        NumCopyToStr(unsigned int, str, uval, padLen);
    }
    else 
    {
        char *ptr = str + 1;
        int len = padLen - 1;
        uval = (unsigned int)(val * -1);
        *str = '-';
        NumCopyToStr(unsigned int, ptr, uval, len);
    }

    return padLen;
}

int NuULongCopyToLPadZero(char *str, unsigned long val, int padLen)
{
    NumCopyToStr(unsigned long, str, val, padLen);
    return padLen;
}

int NuLongCopyToLPadZero(char *str, long val, int padLen)
{
    unsigned long uval;
    if (val >= 0) 
    {
        uval = (unsigned long)val;
        NumCopyToStr(unsigned long, str, uval, padLen);
    }
    else 
    {
        char *ptr = str + 1;
        int len = padLen - 1;
        uval = (unsigned long)(val * -1);
        *str = '-';
        NumCopyToStr(unsigned long, ptr, uval, len);
    }

    return padLen;
}



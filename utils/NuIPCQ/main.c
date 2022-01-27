#include "NuIPCQ.h"

#include <iostream>
#include <string>

const std::string IPCQ_KEY = "0x51f00001";

#define HEXVAL(x)       (((x) >= 'A') ? ((x)-'A'+10) : (x) - '0')
unsigned int str_hex_to_uint(char *szHex) {
	char szHexVal[32] = {0};          /* 0x38000306 */
	int  len = 0;
	unsigned int rlng = 0;
	char *p = NULL;

	strcpy(szHexVal, szHex);

	len = strlen(szHexVal);

	if (len < 3 || len > 10) {
		return -1;
	}

	if ( !(szHexVal[0] == '0' && (szHexVal[1] == 'x' || szHexVal[1] == 'X')) ) {
		return -1;
	}

	for (p = (szHexVal + 2); (p - szHexVal) < len && *p != '\0'; p++) {
		rlng = rlng * 16 + HEXVAL( toupper(*p) );
	}
	return rlng;
}

int main(int argc, char **argv)
{
    int iRC = 0;
    int i = 0;
    int cnt = 100;
	int sz = 1024;

    int action = 0; /* deQ = 0, enQ = 1 */

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-c")) {
			++i;
            cnt = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-s")) {
            ++i;
            sz = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-m")) {
            ++i;
            if (!strcmp(argv[i], "c")) {
                action = 1;
            } else if (!strcmp(argv[i], "S")) {
				action = -1;
			}
        } else if (!strcmp(argv[i], "-h")) {
			Usage(argv[0]);
			goto EXIT;
		}
    }

    printf("mod = %s, cnt = %d, size = %d\n", (action == 1) ? "enQ" : "deQ", cnt, sz);

    // --------------------------------------------------
	NuIPCQ_t   *q = NULL;
	key_t    qKey = str_hex_to_uint(IPCKEY);

	iRC = NuIPCQNew(&q);
	if (iRC < 0){
		printf("err : ipcq new fail. rc = %d\n", iRC);
		goto EXIT;
	}

	iRC = NuIPCQCreate(q, &qKey);
	if (iRC < 0){
		printf("err : ipcq create fail. rc = %d\n", iRC);
		goto EXIT;
	}

    switch(action) {
        case 0:
            deQ(q, sz, cnt, action);
            break;
        case 1:
            enQ(q, sz, cnt);
            break;
        default:
            break;
    }


	if (q != NULL) {
		NuIPCQFree(q);
	}

EXIT:
	return 0;
}
}

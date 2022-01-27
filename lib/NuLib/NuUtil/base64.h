/**************************************************
 **************************************************/

#ifndef _BASE64_H
#define _BASE64_H

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <ctype.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int b64_ntop(u_char const *src, size_t srclength, char *target, size_t targsize) ;
int b64_pton( char const *src, u_char *target, size_t targsize);
#define BASE64_EN(src, src_len, target, target_len) b64_ntop((u_char *)(src), (src_len), (target), (target_len))
#define BASE64_DE(src, target, target_len) b64_pton((src), (u_char *)(target), (target_len))

#ifdef __cplusplus
}
#endif

#endif /* _BASE64_H */


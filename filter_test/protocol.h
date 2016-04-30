#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#define MAXL 1024

struct JobMsg {
	char title[MAXL];
	char options[MAXL];
	char copies[MAXL];
};

void job_msg_decode(char *buf, struct JobMsg *job_msg);
uint32_t job_msg_encode(char *buf, struct JobMsg *job_msg);

#endif

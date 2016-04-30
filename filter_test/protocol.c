#include "protocol.h"
#include "code_engine.h"

uint32_t job_msg_encode(char *buf, struct JobMsg *job_msg) {
    uint32_t rlen = 0;
    rlen += encode_string(&buf, job_msg->title, MAXL);
    rlen += encode_string(&buf, job_msg->options, MAXL);
    rlen += encode_string(&buf, job_msg->copies, MAXL);
    return rlen;
}

void job_msg_decode(char *buf, struct JobMsg *job_msg) {
    decode_string(&buf, job_msg->title, MAXL);
    decode_string(&buf, job_msg->options, MAXL);
    decode_string(&buf, job_msg->copies, MAXL);
}


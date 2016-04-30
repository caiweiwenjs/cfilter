#ifndef __CODE_ENGINE_H__
#define __CODE_ENGINE_H__

#include <stdint.h>

uint32_t decode_int32(char **buf, uint32_t *val);
uint32_t encode_int32(char **buf, uint32_t val);
uint32_t decode_string(char **buf, char *str, uint32_t max);
uint32_t encode_string(char **buf, char *str, uint32_t max);

#endif

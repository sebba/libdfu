#ifndef __DFU_HOST_H__
#define __DFU_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HOST_esp8266
#include "dfu-esp8266.h"
#elif defined HOST_linux
#include "dfu-linux.h"
#else
#error "HOST is not defined"
#endif

static inline uint32_t cpu_to_be32(uint32_t v)
{
#if BYTE_ORDER == BIG_ENDIAN
	return v;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return __builtin_bswap32 (v);
#else
#error "BYTE ORDER is NOT DEFINED !"
#endif
}

static inline uint16_t be32_to_cpu(uint32_t v)
{
	return cpu_to_be32(v);
}

#ifdef __cplusplus
}
#endif

#endif /* __DFU_HOST_H__ */


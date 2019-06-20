#pragma once

#include <string.h>
#include "time.h"
#include <sys/timeb.h>

#ifndef byte
typedef unsigned char			byte;
#endif

#ifdef _WIN32

#ifdef WIN32
#ifndef int8_t
typedef __int8                  int8_t;
#endif
#ifndef int16_t
typedef __int16                 int16_t;
#endif
#ifndef int32_t
typedef __int32                 int32_t;
#endif
#ifndef int64_t
typedef __int64                 int64_t;
#endif

#ifndef uint8_t
typedef unsigned __int8         uint8_t;
#endif
#ifndef uint16_t
typedef unsigned __int16        uint16_t;
#endif
#ifndef uint32_t
typedef unsigned __int32        uint32_t;
#endif
#ifndef uint64_t
typedef unsigned __int64        uint64_t;
#endif
#endif

#ifdef linux
typedef short					int16_t;
typedef int						int32_t;
typedef long long				int64_t;

typedef unsigned char			uint8_t;
typedef unsigned short			uint16_t;
typedef unsigned int			uint32_t;
typedef unsigned long long		uint64_t;
#endif
#else 
#include <stdint.h>


#endif 

// A program stream pack header (not including the system header packets)
struct PS_pack_header
{
	int       id;            // A number to identify this packet
	byte      data[10];      // The data excluding the leading 00 00 01 BA
	uint64_t  scr;           // Formed from scr_base and scr_ext
	uint64_t  scr_base;
	uint32_t  scr_extn;
	uint32_t  program_mux_rate;
	int       pack_stuffing_length;
};

#define IS_AUDIO_STREAM_ID(id)  ((id)==0xBD || ((id) >= 0xC0 && (id) <= 0xDF))
#define IS_VIDEO_STREAM_ID(id)  ((id) >= 0xE0 && (id) <= 0xEF)

#ifndef MAX_UNIT_SIZE
#define MAX_UNIT_SIZE (5*1024*1024)
#endif

#ifndef MAX_AUDIO_UNIT_SIZE
#define MAX_AUDIO_UNIT_SIZE (5*10*1024)
#endif

#define TRUE  1
#define FALSE 0

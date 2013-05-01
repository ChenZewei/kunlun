#ifndef KL_COMMON_PROTOCOL_H_
#define KL_COMMON_PROTOCOL_H_

#define KL_COMMON_PKG_LEN_SIZE	8
typedef unsigned char byte;
#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	typedef struct _pkg_header
	{
		byte pkg_len[KL_COMMON_PKG_LEN_SIZE];
		byte cmd;
		byte status;
	}pkg_header, *pkg_header_ptr;
#ifdef __cplusplus
}
#endif //__cplusplus

#include <stdint.h>
#include <stdio.h>
/*
 * @description: pkg_len, the length of the actual msg pkg
                 pkg_ptr, point to the data of message package
				 sock_stream_fd, sign the source of message stream
 */
class pkg_message
{
public:
	pkg_message() : pkg_len(0), \
		pkg_ptr(NULL), sock_stream_fd(-1)
	{
	}
	~pkg_message()
	{
		if(pkg_ptr != NULL)
		{
			delete [] pkg_ptr;
			pkg_ptr = NULL;
		}
	}
	int64_t pkg_len;
	byte* pkg_ptr;
	int sock_stream_fd; 
};

class CSERIALIZER
{
public:
	inline static int buff2int32(const byte* buff_ptr)
	{
		const byte* p;
		int i;

		p = buff_ptr;
		i = ((int)(*(p++))) << 24;
		i += ((int)(*(p++))) << 16;
		i += ((int)(*(p++))) << 8;
		i += (int)(*p);
		return i;
	}

	inline static int64_t buff2int64(const byte* buff_ptr)
	{
		const byte* p;
		int64_t i;

		p = buff_ptr;
		i = ((int64_t)(*(p++))) << 56;
		i += ((int64_t)(*(p++))) << 48;
		i += ((int64_t)(*(p++))) << 40;
		i += ((int64_t)(*(p++))) << 32;
		i += ((int64_t)(*(p++))) << 24;
		i += ((int64_t)(*(p++))) << 16;
		i += ((int64_t)(*(p++))) << 8;
		i += (int64_t)(*p);
		return i;
	}

	inline static void int2buff(const int n, byte* buff_ptr)
	{
		byte* p;

		p = buff_ptr;
		*(p++) = (n >> 24) & 0xFF;
		*(p++) = (n >> 16) & 0xFF;
		*(p++) = (n >> 8) & 0xFF;
		*p = n & 0xFF;
	}

	inline static void long2buff(const int64_t n, byte* buff_ptr)
	{
		byte* p;

		p = buff_ptr;
		*(p++) = (n >> 56) & 0xFF;
		*(p++) = (n >> 48) & 0xFF;
		*(p++) = (n >> 40) & 0xFF;
		*(p++) = (n >> 32) & 0xFF;
		*(p++) = (n >> 24) & 0xFF;
		*(p++) = (n >> 16) & 0xFF;
		*(p++) = (n >> 8) & 0xFF;
		*p = n & 0xFF;
	}
};

#endif //KL_COMMON_PROTOCOL_H_
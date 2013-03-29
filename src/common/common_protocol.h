#ifndef KL_COMMON_PROTOCOL_H_
#define KL_COMMON_PROTOCOL_H_

#define KL_COMMON_PKG_LEN_SIZE	8
typedef unsigned char byte;
typedef struct _pkg_header
{
	byte pkg_len[KL_COMMON_PKG_LEN_SIZE];
	byte cmd;
	byte status;
}pkg_header, *pkg_header_ptr;

#include <stdint.h>
#include <stdio.h>
/*
typedef struct _pkg_message
{
	int64_t pkg_len;
	byte* pkg_ptr;
	void* sock_stream_ptr;
}pkg_message, *pkg_message_ptr;
*/
class pkg_message
{
public:
	pkg_message() : pkg_len(0), \
		pkg_ptr(NULL), msg_stream_ptr(NULL)
	{
	}
	~pkg_message()
	{
		if(pkg_ptr != NULL)
		{
			delete pkg_ptr;
			pkg_ptr = NULL;
		}
	}
	int64_t pkg_len;
	byte* pkg_ptr;
	void* msg_stream_ptr; //point to the source of message stream
};

class CSERIALIZER
{
public:
	inline static int buff2int32(const byte* buff_ptr)
	{
		const byte* p;
		int i;

		p = buff_ptr;
		i = *(p++) << 24;
		i += *(p++) << 16;
		i += *(p++) << 8;
		i += *p;
		return i;
	}

	inline static int64_t buff2int64(const byte* buff_ptr)
	{
		const byte* p;
		int64_t i;

		p = buff_ptr;
		i = *(p++) << 56;
		i += *(p++) << 48;
		i += *(p++) << 40;
		i += *(p++) << 32;
		i += *(p++) << 24;
		i += *(p++) << 16;
		i += *(p++) << 8;
		i += *p;
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
#ifndef KL_COMMON_HASH_H_
#define KL_COMMON_HASH_H_
#include "file.h"
#include "common_protocol.h"
/* Type define */
typedef unsigned int uint32;

/* MD5 declaration. */
class MD5 {
public:
	MD5();
	MD5(const void* in_buf, size_t length);
	/*
	 * @brief: if failed, throw an errcode
	 */
	MD5(CFile *pfile);
	int update(const void* input, size_t length);
	int update(CFile *pfile);
	const byte* digest();
	const char* to_string(char *pout_buf, size_t nsize);
	int reset();

private:
	int update(const byte* input, size_t length);
	int final();
	int transform(const byte block[64]);
	int encode(const uint32* input, byte* output, size_t length);
	int decode(const byte* input, uint32* output, size_t length);
	const char* bytes2hexstring();

	/* class uncopyable */
	MD5(const MD5&);
	MD5& operator=(const MD5&);

private:
	uint32 m_state[4];	/* state (ABCD) */
	uint32 m_count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	byte m_buffer[64];	/* input buffer */
	byte m_digest[16];	/* message digest */
	bool m_bfinished;		/* calculate finished ? */

	static const byte PADDING[64];	/* padding for calculate */
	static const char HEX[16];
};

#endif //KL_COMMON_HASH_H_

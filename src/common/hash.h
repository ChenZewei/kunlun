#ifndef KL_COMMON_HASH_H_
#define KL_COMMON_HASH_H_
#include "file.h"
#include "common_protocol.h"
/* type define */
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

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
	int encode(const uint32_t* input, byte* output, size_t length);
	int decode(const byte* input, uint32_t* output, size_t length);
	const char* bytes2hexstring();

	/* class uncopyable */
	MD5(const MD5&);
	MD5& operator=(const MD5&);

private:
	uint32_t m_state[4];	/* state (ABCD) */
	uint32_t m_count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	byte m_buffer[64];	/* input buffer */
	byte m_digest[16];	/* message digest */
	bool m_bfinished;		/* calculate finished ? */

	static const byte PADDING[64];	/* padding for calculate */
};

class CNameSpaceHash
{
public:
	CNameSpaceHash(int nname_power, int nvnode_count);
	~CNameSpaceHash();

	int get_vnode_id(const char *file_path);
	uint64_t mem_hash(const void *buf, int size);
	uint64_t str_hash(const char *str);
private:
	uint64_t bkdr_hash(const char *str, int size);

	int m_nname_power;
	int m_nvnode_count;
};

#endif //KL_COMMON_HASH_H_

#ifndef KL_COMMON_TYPES_H_
#define KL_COMMON_TYPES_H_

/*
 * @description: set buf size to 4096 just because the size is similar to
                 the size of unix kernel's buffer, even though set buf to 
				 more size, the efficiency of copy isn't promoted
 */
#define KL_COMMON_BUF_SIZE		4096 
#define KL_COMMON_PATH_LEN		256 //the max path length
#define KL_COMMON_MD5_HASH_LEN	33 //the length of md5 hash code
#define KL_COMMON_IP_ADDR_LEN	16	//IPv4
#define KL_COMMON_EXIT_ERR		-1	//system exit with error
#define KL_COMMON_EXIT_SYS		0	//system exit without error

#ifdef USE_SELECT
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#define KL_COMMON_STREAM_EXTEND	0
#else
#ifdef USE_POLL
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#define KL_COMMON_STREAM_EXTEND	0
#else //default : USE_EPOLL
#define KL_COMMON_STREAM_IN EPOLLIN
#define KL_COMMON_STREAM_OUT EPOLLOUT
#define KL_COMMON_STREAM_EXTEND	EPOLLET
#endif //USE_POLL
#endif //USE_SELECT

#include <new>
#define kl_new new(std::nothrow)

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	static void kl_errout(const char *str)
	{
		printf("%s\n", str);
		_exit(KL_COMMON_EXIT_ERR);
	}

	/*
	 * @brief: convert data from binary to hexadecimal
	 * @return: if successed, return the poionter of phex_buf,
	            otherwise, return NULL
	 */
	static char* kl_bin2hex(char *phex_buf, int nhex_size, \
		const char *pbin_buf, int nbin_size)
	{
		int i;
		unsigned char n;
		unsigned char low;
		unsigned char high;

		memset(phex_buf, 0, nhex_size);
		if(nhex_size < nbin_size * 2 + 1)
		{
			return NULL;
		}

		for(i = 0; i < nbin_size; i++)
		{
			//convert the low 4 bit
			n = (unsigned char)pbin_buf[i];
			low = n & 0x0F;
			phex_buf[2 * i + 1] = (char)(low < 0x0A ? low + '0' : low - 0x0A + 'A');
			//convert the high 4 bit
			high = (n >> 4) & 0x0F;
			phex_buf[2 * i] = (char)(high < 0x0A ? high + '0' : high - 0x0A + 'A');
		}
		phex_buf[2 * i] = '\0';
		return phex_buf;
	}

	/*
	 * @brief: convert data from hexadecimal to binary 
	 * @return: if successed, return the poionter of pbin_buf,
	            otherwise, return NULL
	 */
	static char* kl_hex2bin(char *pbin_buf, int nbin_size, \
		const char *phex_buf, int nhex_size)
	{
		int i;
		unsigned char low;
		unsigned char high;

		memset(pbin_buf, 0, nbin_size);
		if(nhex_size < nbin_size * 2)
		{
			return NULL;
		}
		for(i = 0; i < nbin_size; i++)
		{
			//convert the high 4 bit
			if(phex_buf[2 * i] >= 'A' && phex_buf[2 * i] <= 'F')
			{
				high = (unsigned char)(phex_buf[2 * i] + 0x0A - 'A');
			}
			else if(phex_buf[2 * i] >= '0' && phex_buf[2 * i] <= '9')
			{
				high = (unsigned char)(phex_buf[2 * i] - '0');
			}
			else
			{
				return NULL;
			}
			pbin_buf[i] = (char)(high << 4);
			//convert the low 4 bit
			if(phex_buf[2 * i + 1] >= 'A' && phex_buf[2 * i + 1] <= 'F')
			{
				low = (unsigned char)(phex_buf[2 * i + 1] + 0x0A - 'A');
			}
			else if(phex_buf[2 * i + 1] >= '0' && phex_buf[2 * i + 1] <= '9')
			{
				low = (unsigned char)(phex_buf[2 * i + 1] - '0');
			}
			else
			{
				return NULL;
			}
			pbin_buf[i] = (char)((unsigned char)pbin_buf[i] | low);
		}
		return pbin_buf;
	}
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //KL_COMMON_TYPES_H_
#ifndef KL_COMMON_FILE_READER_STREAM_H_
#define KL_COMMON_FILE_READER_STREAM_H_
#include "file.h"
#include "sockstream.h"
class CFileReaderStream : public CFile
{
public:
	CFileReaderStream(const char *path, CSockStream *psock_stream);
	~CFileReaderStream();

	int stream_readout();
private:
	CSockStream *m_psock_stream;
};
#endif //KL_COMMON_FILE_READER_STREAM_H_
#ifndef KL_COMMON_FILE_WRITER_STREAM_H_
#define KL_COMMON_FILE_WRITER_STREAM_H_
#include "file.h"
#include "sockstream.h"
class CFileWriterStream : public CFile
{
public:
	CFileWriterStream(const char *path, CSockStream *psock_stream);
	~CFileWriterStream();

	int stream_writein(int64_t nfile_size);
private:
	CSockStream *m_psock_stream;
};
#endif //KL_COMMON_FILE_WRITER_STREAM_H_
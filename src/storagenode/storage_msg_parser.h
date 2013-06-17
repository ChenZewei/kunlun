#ifndef KL_STORAGE_MSG_PARSER_H_
#define KL_STORAGE_MSG_PARSER_H_
#include "msg_parser.h"
#include "storage_protocol.h"
class CStorageMsgParser : public CMsgParser
{
public:
	CStorageMsgParser();
	~CStorageMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);
private:
	int msg_file_check_handle(pkg_message *pkg_msg_ptr);
	int msg_file_sync_handle(pkg_message *pkg_msg_ptr);
	int msg_upload_file_handle(pkg_message *pkg_msg_ptr);
	int msg_download_file_handle(pkg_message *pkg_msg_ptr);
	int msg_ctrl_stream_handle(pkg_message *pkg_msg_ptr);
	/*
	 * @brief: check file hashcode 
	 * @param: bsync_flag, if check hash code different, need to sync file,
	           set bsync_flag to true, otherwise, set bsync_flag to flase
	 */
	int do_file_hashcode_check(pcheck_file_info file_info_ptr, bool &bsync_flag);
	int do_db_file_check(const char *file_path, const char *hashcode, bool &bsync_flag);
	int do_data_file_check(const char *file_path, const char *hashcode, bool &bsync_flag);
};
#endif //KL_STORAGE_MSG_PARSER_H_
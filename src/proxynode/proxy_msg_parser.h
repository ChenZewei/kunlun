#ifndef KL_PROXY_MSG_PARSER_H_
#define KL_PROXY_MSG_PARSER_H_
#include "node_info.h"
#include "msg_parser.h"
class CProxyMsgParser : public CMsgParser
{
public:
	CProxyMsgParser();
	~CProxyMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);

	int msg_device_join_handle(pkg_message* pkg_msg_ptr);
	int msg_device_report_handle(pkg_message* pkg_msg_ptr);
private:
	/*
	 * @brief: do device joining operation when the count of node less than
	           the count of replica
	 * @param: pdevice_info, the device info will be joined to node container
	 * @return: if successed, return 0, otherwise, return -1
	 */
	int do_device_join1(device_info_ptr pdevice_info);
	/*
	 * @brief: do device joining operation when the count of node larger than
	           the count of replica
	 * @param: pdevice_info, the device info will be joined to node container
	 * @return: if successed, return 0, otherwise, return -1
	 */
	int do_device_join2(device_info_ptr pdevice_info);
};
#endif //KL_PROXY_MSG_PARSER_H_
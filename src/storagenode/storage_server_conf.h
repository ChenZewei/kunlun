#ifndef KL_STORAGE_SERVER_CONF_H_
#define KL_STORAGE_SERVER_CONF_H_
#include <vector>
#include "inetaddr.h"
#include "common_types.h"
#include "base_server_conf.h"

typedef std::vector<CInetAddr> addr_list;
class CStorageServerConf : public CBaseServerConf
{
public:
	//zone id
	int nzone_id;
	// the weight of storage node
	int nweight;
	//the root path of device
	char device_path[KL_COMMON_PATH_LEN];
	//the list of proxy address
	 addr_list proxy_addr_list;
};
#endif //KL_STORAGE_SERVER_CONF_H_
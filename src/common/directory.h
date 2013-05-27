#ifndef KL_COMMON_DIRECTORY_H_
#define KL_COMMON_DIRECTORY_H_
class CDirectory
{
public:
	CDirectory();
	~CDirectory();

	int make_dir(const char *dir_path);
	/*
	 * @brief: if the removed dir have file or child dir,
	           function will delete file or child dir
	   @return: if successed, return 0, otherwise, return errcode
	 */
	int remove_dir(const char *dir_path);
	bool dir_exist(const char *dir_path);
};
#endif //KL_COMMON_DIRECTORY_H_
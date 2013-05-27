#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"
#include "directory.h"
#include "common_types.h"

CDirectory::CDirectory()
{

}

CDirectory::~CDirectory()
{

}

int CDirectory::make_dir(const char *dir_path)
{
	if(dir_path == NULL || strlen(dir_path) == 0)
	{
		return 0;
	}
	if(access(dir_path, F_OK) == 0)
	{
		return 0;
	}

	int ret;
	const char *p;
	char parent_dir[KL_COMMON_PATH_LEN];

	p = strrchr(dir_path, '/');
	if(p != NULL)
	{
		//make parent dir
		memset(parent_dir, 0, KL_COMMON_PATH_LEN);
		memcpy(parent_dir, dir_path, p - dir_path);
		if(ret = make_dir(parent_dir) != 0)
		{
			return ret;
		}
	}

	if(access(dir_path, F_OK) == 0)
	{
		return 0;
	}
	if(mkdir(dir_path, 0755) != 0)
	{
		errno = errno != 0 ? errno : EINVAL;
		if(g_psys_log == NULL)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"call mkdir failed, dir_path: %s, err: %s\n", \
				__LINE__, dir_path, strerror(errno));
		}
		else
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call mkdir failed, dir_path: %s, err: %s", \
				__LINE__, dir_path, strerror(errno));
		}
		return errno;
	}
	return 0;
}

int CDirectory::remove_dir(const char *dir_path)
{
	if(dir_path == NULL || strlen(dir_path) == 0)
	{
		return EINVAL;
	}
	if(access(dir_path, F_OK) != 0)
	{
		errno = errno != 0 ? errno : EINVAL;
		if(g_psys_log == NULL)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"call access failed, dir_path: %s, err: %s\n", \
				__LINE__, dir_path, strerror(errno));
		}
		else
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call access failed, dir_path: %s, err: %s", \
				__LINE__, dir_path, strerror(errno));
		}
		return errno;
	}

	int ret;
	DIR *pdir;
	struct dirent entry;
	struct dirent *presult;
	char child_path[KL_COMMON_PATH_LEN];

	if((pdir = opendir(dir_path)) == NULL)
	{
		errno = errno != 0 ? errno : EINVAL;
		if(g_psys_log == NULL)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"call opendir failed, dir_path: %s, err: %s\n", \
				__LINE__, dir_path, strerror(errno));
		}
		else
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call opendir failed, dir_path: %s, err: %s", \
				__LINE__, dir_path, strerror(errno));
		}
		return errno;
	}

	while((readdir_r(pdir, &entry, &presult) == 0) && (presult != NULL))
	{
		memset(child_path, 0, KL_COMMON_PATH_LEN);
		if(dir_path[strlen(dir_path) - 1] == '/')
		{
			snprintf(child_path, KL_COMMON_PATH_LEN, "%s%s", \
				dir_path, entry.d_name);
		}
		else
		{
			snprintf(child_path, KL_COMMON_PATH_LEN, "%s/%s", \
				dir_path, entry.d_name);
		}

		if(entry.d_type & DT_DIR)
		{
			if(strcmp(entry.d_name, ".") == 0 || \
				strcmp(entry.d_name, "..") == 0)
				continue;
			if((ret = remove_dir(child_path)) != 0)
			{
				closedir(pdir);
				return ret;
			}
			continue;
		}

		//delete files in this dir
		if(unlink(child_path) != 0)
		{
			errno = errno != 0 ? errno : EINVAL;
			if(g_psys_log == NULL)
			{
				fprintf(stderr, "file: "__FILE__", line: %d, " \
					"call unlink failed, dir_path: %s, err: %s\n", \
					__LINE__, child_path, strerror(errno));
			}
			else
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"call unlink failed, dir_path: %s, err: %s", \
					__LINE__, child_path, strerror(errno));
			}
			closedir(pdir);
			return errno;
		}
	}
	//now remove this dir
	if(rmdir(dir_path) != 0)
	{
		errno = errno != 0 ? errno : EINVAL;
		if(g_psys_log == NULL)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"call rmdir failed, dir_path: %s, err: %s\n", \
				__LINE__, dir_path, strerror(errno));
		}
		else
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call rmdir failed, dir_path: %s, err: %s", \
				__LINE__, dir_path, strerror(errno));
		}
		closedir(pdir);
		return errno;
	}
	closedir(pdir);
	return 0;
}

bool CDirectory::dir_exist(const char *dir_path)
{
	if(access(dir_path, F_OK) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
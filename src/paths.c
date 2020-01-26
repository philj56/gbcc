#include "paths.h"

#ifdef __WIN32
#include <direct.h>
#include <windows.h>

#define MAX_PATH_LENGTH 4096

/*
 * If anyone knows of a better way to achieve this,
 * I'd be happy to hear it.
 */
void gbcc_fix_windows_path()
{
	char *path = calloc(1, MAX_PATH_LENGTH);
	GetModuleFileName(NULL, path, MAX_PATH_LENGTH - 1);
	for (char *c = path + MAX_PATH_LENGTH - 1; *c != '\\'; c--) {
		*c = '\0';
	}
	_chdir(path);
}
#else
void gbcc_fix_windows_path() {}
#endif /* __WIN32 */

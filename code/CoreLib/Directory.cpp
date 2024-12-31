#include "Directory.h"
#include <hook.h>

MH_DefineFunc(DirectoryCreate, 0x0057ae94, TOC1, void, CFilePath& src);
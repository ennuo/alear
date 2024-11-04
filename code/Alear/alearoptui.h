#ifndef ALEAR_OPT_UI_H
#define ALEAR_OPT_UI_H

#include <filepath.h>

void InitAlearOptUiHooks();
void OnDatabaseFileChanged(CFilePath& fp);
void OnUpdateLevel();

#endif // ALEAR_OPT_UI_H
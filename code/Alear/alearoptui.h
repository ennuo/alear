#ifndef ALEAR_OPT_UI_H
#define ALEAR_OPT_UI_H

#include <filepath.h>

void InitAlearOptUiHooks();
void OnDatabaseFileChanged(CFilePath& fp);
void ReloadPendingDatabases();

#endif // ALEAR_OPT_UI_H
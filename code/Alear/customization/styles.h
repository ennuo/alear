#ifndef STYLE_CORE_H
#define STYLE_CORE_H

void InitPopitStyleHooks();
void InitSlapStyleHooks();
void InitEmoteHooks();

void InitStyleHooks();

extern bool LoadSlapStyles();
extern void UnloadSlapStyles();

extern bool LoadEmotes();
extern void UnloadEmotes();

#endif // STYLE_CORE_H
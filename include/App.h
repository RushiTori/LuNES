#ifndef APP_H
#define APP_H

#include "Common.h"

typedef struct App {
	/* data */
	Color cursorCol;
	bool shouldClose;
} App;

App* AppGetSingleton(void);

bool AppInit(void);
bool AppShouldClose(void);
void AppClose(void);

void AppUpdate(void);
void AppDisplay(void);

#endif	// APP_H

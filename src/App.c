#include "App.h"

App singleton = (App){0};

App* AppGetSingleton(void) { return &singleton; }

bool AppInit(void) {
	singleton.cursorCol = (Color){.r = 255, .g = 109, .b = 194, .a = 255};
	return true;
}

bool AppShouldClose(void) { return singleton.shouldClose || WindowShouldClose(); }

void AppClose(void) { singleton = (App){0}; }

static void ChangeCol(void) {
	singleton.cursorCol.r = rand() % UINT8_MAX;
	singleton.cursorCol.g = rand() % UINT8_MAX;
	singleton.cursorCol.b = rand() % UINT8_MAX;
}

void AppUpdate(void) {
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ChangeCol();
	if (WindowShouldClose()) singleton.shouldClose;
}

void AppDisplay(void) {
	BeginDrawing();
	ClearBackground(DARKBLUE);

	DrawCircleV(GetMousePosition(), 15, GOLD);
	DrawCircleV(GetMousePosition(), 12, singleton.cursorCol);

	EndDrawing();
}

#include "main.h"

static bool RaylibInit(void) {
	TraceLogLevel minLogLevel = LOG_WARNING;
	SetTraceLogLevel(minLogLevel);

	float winRatio = 2.0f;
	uint32_t width = 1920 / winRatio;
	uint32_t height = 1080 / winRatio;

	const char* title = "LuNES Emulator - made with Raylib";
	InitWindow(width, height, title);
	if (!IsWindowReady()) return false;

	ConfigFlags flags = 0;
	flags |= FLAG_VSYNC_HINT;		  // Set to try enabling V-Sync on GPU
	flags |= FLAG_WINDOW_RESIZABLE;	  // Set to allow resizable window
	flags |= FLAG_WINDOW_ALWAYS_RUN;  // Set to allow windows running while minimized
	SetConfigFlags(flags);

	int32_t fps = 60;
	SetTargetFPS(fps);

	InitAudioDevice();
	if (!IsAudioDeviceReady()) {
		CloseWindow();
		return false;
	}

	return true;
}

static void RaylibClose(void) {
	CloseAudioDevice();
	CloseWindow();
}

int main(void) {
	if (!RaylibInit()) exit(EXIT_FAILURE);

	if (!AppInit()) {
		RaylibClose();
		exit(EXIT_FAILURE);
	}

	while (!AppShouldClose()) {
		AppUpdate();
		AppDisplay();
	}

	AppClose();
	RaylibClose();
	exit(EXIT_SUCCESS);
}

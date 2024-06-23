#include <SDL3/SDL.h>
#include <SDL3/SDL_pen.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// Function to initialize SDL
int init(SDL_Window **window, SDL_Renderer **renderer) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	*window = SDL_CreateWindow("SDL Test", 1280, 720, 0);
	if (*window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	*renderer = SDL_CreateRenderer(*window, NULL, 0);
	if (*renderer == NULL) {
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	int pen_count;
	SDL_PenID* pens = SDL_GetPens(&pen_count);
	for (int i = 0; i < pen_count; i++) {
		printf("Pen: %d\n", pens[i]);
	}

	return 0;
}

// Function to handle pen events
bool handlePenEvent(SDL_Event *event) {
	switch (event->type) {
		case SDL_EVENT_PEN_DOWN:
			printf("Pen button pressed\n");
			return true;
		case SDL_EVENT_PEN_UP:
			printf("Pen button released\n");
			return true;
		case SDL_EVENT_PEN_MOTION:
			printf("Pen pressure: %f\n", event->pmotion.axes[SDL_PEN_AXIS_PRESSURE]);
			return true;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			switch (event->button.button) {
				case SDL_BUTTON_LEFT:
					printf("Left Mouse\n");
					return true;
				case SDL_BUTTON_MIDDLE:
					printf("Middle Mouse\n");
					return true;
				case SDL_BUTTON_RIGHT:
					printf("Right Mouse\n");
					return true;
			}
		case SDL_EVENT_MOUSE_MOTION:
//			printf("Mouse Motion: %d x %d\n", (int)event->motion.x, (int)event->motion.y);
//			return true;
		default:
			return false;
	}
}


int main(int argc, char *argv[]) {
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	srand(time(NULL));

	if (init(&window, &renderer) < 0) {
		printf("Failed to initialize!\n");
		return -1;
	}

	const char* path = "/Users/kiki/dev/animatorpro/src/resource/font";
	int count = 0;
	char** files = SDL_GlobDirectory(path, "*", SDL_GLOB_CASEINSENSITIVE, &count);

	SDL_PathInfo info;
	char full_path[1024];

	for (int i = 0; i < count; i++) {
		snprintf(full_path, 1024, "%s/%s", path, files[i]);
		if (SDL_GetPathInfo(full_path, &info) != 0) {
			fprintf(stderr, "-- Error on %s: %s\n", files[i], SDL_GetError());
			continue;
		}

		const char* path_type = info.type == SDL_PATHTYPE_DIRECTORY ? "D" : "F";

		fprintf(stderr, "W: %s (%s)\n", files[i], path_type);
	}

	SDL_Event event;
	int quit = 0;

	SDL_Color color = { .r=0xFF, .g=0xFF, .b=0xFF, .a=0xFF };

	while (!quit) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_EVENT_QUIT) {
				quit = 1;
			} else {
				if (handlePenEvent(&event)) {
					color.r = rand() % 0xFF;
					color.g = rand() % 0xFF;
					color.b = rand() % 0xFF;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
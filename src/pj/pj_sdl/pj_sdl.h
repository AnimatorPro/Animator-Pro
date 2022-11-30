//
// Created by Charles Wardlaw on 2022-10-02.
//

#ifndef ANIMATOR_PRO_PJ_SDL_H
#define ANIMATOR_PRO_PJ_SDL_H

/*--------------------------------------------------------------*/
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Rect SDL_Rect;

extern SDL_Surface*  s_surface;
extern SDL_Surface*  s_buffer;
extern SDL_Window*   window;
extern SDL_Surface*  s_window_surface;
extern SDL_Renderer* renderer;

int pj_sdl_get_video_size(LONG* width, LONG* height);
int pj_sdl_get_window_size(LONG* width, LONG* height);
int pj_sdl_get_window_scale(float* x, float* y);
LONG pj_sdl_get_display_scale(void);
SDL_Rect pj_sdl_fit_surface(const SDL_Surface* rect, const SDL_Surface* target);
void pj_sdl_flip_window_surface(void);

#ifdef __APPLE__
const char* mac_resources_path();
const char* mac_preferences_path();
#endif // __APPLE__

#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif

#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

#endif // ANIMATOR_PRO_PJ_SDL_H

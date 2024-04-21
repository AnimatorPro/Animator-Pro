//
// Created by Charles Wardlaw on 2022-10-02.
//

#ifndef ANIMATOR_PRO_PJ_SDL_H
#define ANIMATOR_PRO_PJ_SDL_H

/*--------------------------------------------------------------*/
struct SDL_Surface;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Rect;

extern struct SDL_Surface*  s_surface;
extern struct SDL_Surface*  s_buffer;
extern struct SDL_Window*   window;
extern struct SDL_Surface*  s_window_surface;
extern struct SDL_Renderer* renderer;

int pj_sdl_get_video_size(LONG* width, LONG* height);
int pj_sdl_get_window_size(LONG* width, LONG* height);
int pj_sdl_get_window_scale(float* x, float* y);
LONG pj_sdl_get_display_scale(void);
struct SDL_Rect pj_sdl_fit_surface(const struct SDL_Surface* rect, const struct SDL_Surface* target);
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

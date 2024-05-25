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
extern struct SDL_Texture* render_target;

int pj_sdl_get_video_size(LONG* width, LONG* height);
int pj_sdl_get_window_size(LONG* width, LONG* height);
int pj_sdl_get_window_scale(float* x, float* y);
LONG pj_sdl_get_display_scale(void);
struct SDL_FRect pj_sdl_fit_surface(const struct SDL_Surface* source, int target_w, int target_h);
void pj_sdl_flip_window_surface(void);

#ifdef SDL_PLATFORM_APPLE
const char* mac_resources_path();
const char* mac_preferences_path();
#endif // SDL_PLATFORM_APPLE

#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif

#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

#endif // ANIMATOR_PRO_PJ_SDL_H

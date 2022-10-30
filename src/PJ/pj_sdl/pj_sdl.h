//
// Created by Charles Wardlaw on 2022-10-02.
//

#ifndef ANIMATOR_PRO_PJ_SDL_H
#define ANIMATOR_PRO_PJ_SDL_H

/*--------------------------------------------------------------*/
extern SDL_Surface*  s_surface;
extern SDL_Surface*  s_buffer;
extern SDL_Window*   window;
extern SDL_Surface*  s_window_surface;
extern SDL_Renderer* renderer;

int pj_sdl_get_video_size(LONG* width, LONG* height);
LONG pj_sdl_get_display_scale();

#endif // ANIMATOR_PRO_PJ_SDL_H

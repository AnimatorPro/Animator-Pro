//
// Created by Charles Wardlaw on 2022-10-02.
//

#ifndef ANIMATOR_PRO_PJ_SDL_H
#define ANIMATOR_PRO_PJ_SDL_H

/*--------------------------------------------------------------*/
extern SDL_Surface *s_surface;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;


int pj_sdl_get_video_size(LONG* width, LONG* height);


#endif // ANIMATOR_PRO_PJ_SDL_H

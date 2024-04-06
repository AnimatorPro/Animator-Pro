//
// Created by Charles Wardlaw on 2024-04-06.
//

#ifndef ANIMATOR_PRO_POCODRAW_H
#define ANIMATOR_PRO_POCODRAW_H

#include "stdtypes.h"
#include "pocolib.h"

void po_ink_line(int x1, int y1, int x2, int y2);
void po_hls_to_rgb(int h, int l, int s, Popot r, Popot g, Popot b);
void po_rgb_to_hls(int r, int g, int b, Popot h, Popot l, Popot s);
int po_closest_color_in_screen(Popot screen, int r, int g, int b);
ErrCode po_squeeze_colors(Popot source_map, int source_count, Popot dest_map, int dest_count);
Errcode po_fit_screen_to_color_map(Popot screen, Popot new_colors, bool keep_key);


#endif // ANIMATOR_PRO_POCODRAW_H

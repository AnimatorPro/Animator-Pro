//
// Created by Charles Wardlaw on 2024-04-06.
//

#ifndef ANIMATOR_PRO_POCO_TWEEN_H
#define ANIMATOR_PRO_POCO_TWEEN_H

#include "stdtypes.h"


void init_poco_tween(void);
void cleanup_poco_tween(void);

#ifdef POCO_TWEEN_INTERNALS
static bool po_tween_exists(void);
static void po_tween_clear_links(void);
#endif

#endif // ANIMATOR_PRO_POCO_TWEEN_H

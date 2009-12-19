#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL/SDL.h>
#include "common.h"
#include "handlers.h"
#include "gmath.h"

/// input namespace
namespace i {


extern void Reset();
extern void HandleEvents();

// keys and btns
extern short keys [];  ///< Shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continucely
extern short mouse_btns [];    ///< Mouse btns. Supporting 3 btns & wheel. Works the same as above

// mouse stuff
extern vec2_t mouse_pos_ndc; ///< The coords are in the NDC space
extern vec2_t mouse_pos;     ///< The coords are in the window space. (0,0) is in the uper left corner
extern vec2_t mouse_velocity;
extern bool warp_mouse;
extern bool hide_cursor;



} // end namespace
#endif
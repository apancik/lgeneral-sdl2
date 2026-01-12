/***************************************************************************
                          event.c  -  description
                             -------------------
    begin                : Wed Mar 20 2002
    copyright            : (C) 2001 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lgeneral.h"
#include "event.h"
#include "lg-sdl.h"

int buttonup = 0, buttondown = 0;
int motion = 0;
int motion_x = 50, motion_y = 50;
static Uint8 keystate_map[SDL_NUM_SCANCODES];
int buttonstate[10];
int sdl_quit = 0;

static void normalize_mouse_pos(int wx, int wy, int *lx, int *ly)
{
#if SDL_VERSION_ATLEAST(2,0,18)
    if (sdl.renderer) {
        float fx = 0.0f, fy = 0.0f;
        SDL_RenderWindowToLogical(sdl.renderer, (float)wx, (float)wy, &fx, &fy);
        *lx = (int)(fx + 0.5f);
        *ly = (int)(fy + 0.5f);
        return;
    }
#endif
    if (sdl.input_scale_x > 0.0 && sdl.input_scale_y > 0.0) {
        *lx = (int)( (double)wx / sdl.input_scale_x + 0.5 );
        *ly = (int)( (double)wy / sdl.input_scale_y + 0.5 );
        return;
    }
    *lx = wx;
    *ly = wy;
}

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Event filter that blocks all events. Used to clear the SDL
event queue.
====================================================================
*/
int all_filter( void *userdata, SDL_Event *event )
{
    return 0;
}

/*
====================================================================
Returns whether any mouse button is pressed
====================================================================
*/
static int event_is_button_pressed()
{
    return (SDL_GetMouseState(NULL, NULL) != 0);
}
static int event_is_key_pressed()
{
    const Uint8 *ks = SDL_GetKeyboardState(NULL);
    int i;
    for (i = 0; i < SDL_NUM_SCANCODES; i++) {
        if (ks[i]) return 1;
    }
    return 0;
}

/*
====================================================================
Event filter. As long as this is active no KEY or MOUSE events
will be available by SDL_PollEvent().
====================================================================
*/
int event_filter( void *userdata, SDL_Event *event )
{
    switch ( event->type ) {
        case SDL_MOUSEMOTION:
            normalize_mouse_pos(event->motion.x, event->motion.y, &motion_x, &motion_y);
            buttonstate[BUTTON_LEFT] = (event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
            buttonstate[BUTTON_MIDDLE] = (event->motion.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0;
            buttonstate[BUTTON_RIGHT] = (event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0;
            motion = 1;
            return 0;
        case SDL_MOUSEBUTTONUP:
            if ( event->button.button >= 0 && event->button.button < 10 ) buttonstate[event->button.button] = 0;
            normalize_mouse_pos(event->button.x, event->button.y, &motion_x, &motion_y);
            buttonup = event->button.button;
            return 0;
        case SDL_MOUSEBUTTONDOWN:
            if ( event->button.button >= 0 && event->button.button < 10 ) buttonstate[event->button.button] = 1;
            normalize_mouse_pos(event->button.x, event->button.y, &motion_x, &motion_y);
            buttondown = event->button.button;
            return 0;
        case SDL_KEYUP: {
            SDL_Scancode sc = event->key.keysym.scancode;
            if ( sc >= 0 && sc < SDL_NUM_SCANCODES ) keystate_map[sc] = 0;
            return 0;
        }
        case SDL_KEYDOWN: {
            SDL_Scancode sc = event->key.keysym.scancode;
            if ( sc >= 0 && sc < SDL_NUM_SCANCODES ) keystate_map[sc] = 1;
            return 1;
        }
        case SDL_QUIT:
            sdl_quit = 1;
            return 0;
    }
    return 1;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Enable/Disable event filtering of mouse and keyboard.
====================================================================
*/
void event_enable_filter( void )
{
    SDL_SetEventFilter( event_filter, NULL );
    event_clear();
}
void event_disable_filter( void )
{
    SDL_SetEventFilter( NULL, NULL );
}
void event_clear( void )
{
    memset( keystate_map, 0, sizeof( keystate_map ) );
    memset( buttonstate, 0, sizeof( int ) * 10 );
    buttonup = buttondown = motion = 0;
}

/*
====================================================================
Check if an motion event occured since last call.
====================================================================
*/
int event_get_motion( int *x, int *y )
{
    if ( motion ) {
        *x = motion_x;
        *y = motion_y;
        motion = 0;
        return 1;
    }
    return 0;
}
/*
====================================================================
Check if a button was pressed and return its value.
====================================================================
*/
int event_get_buttondown( int *button, int *x, int *y )
{
    if ( buttondown ) {
        *button = buttondown;
        *x = motion_x;
        *y = motion_y;
        buttondown = 0;
        return 1;
    }
    return 0;
}
int event_get_buttonup( int *button, int *x, int *y )
{
    if ( buttonup ) {
        *button = buttonup;
        *x = motion_x;
        *y = motion_y;
        buttonup = 0;
        return 1;
    }
    return 0;
}

/*
====================================================================
Get current cursor position.
====================================================================
*/
void event_get_cursor_pos( int *x, int *y )
{
    *x = motion_x; *y = motion_y;
}

/*
====================================================================
Check if 'button' button is currently set.
====================================================================
*/
int event_check_button( int button )
{
    return buttonstate[button];
}

/*
====================================================================
Check if 'key' is currently set.
====================================================================
*/
int event_check_key( int key )
{
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    if ( sc >= 0 && sc < SDL_NUM_SCANCODES )
        return keystate_map[sc];
    return 0;
}

/*
====================================================================
Check if SDL_QUIT was received.
====================================================================
*/
int event_check_quit()
{
    return sdl_quit;
}

/*
====================================================================
Clear the SDL event key (keydown events)
====================================================================
*/
void event_clear_sdl_queue()
{
    SDL_Event event;
    SDL_SetEventFilter( all_filter, NULL );
    while ( SDL_PollEvent( &event ) );
    SDL_SetEventFilter( event_filter, NULL );
}

/*
====================================================================
Wait until neither a key nor a button is pressed.
====================================================================
*/
void event_wait_until_no_input()
{
    SDL_PumpEvents();
    while (event_is_button_pressed()||event_is_key_pressed())
    {
        SDL_Delay(20);
        SDL_PumpEvents();
    }
    event_clear();
}

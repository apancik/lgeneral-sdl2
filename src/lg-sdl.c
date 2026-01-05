/***************************************************************************
                          lg-sdl  -  description
                             -------------------
    begin                : Thu Apr 20 2000
    copyright            : (C) 2000 by Michael Speck
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

#include "lg-sdl.h"

#include "misc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "localize.h"

extern int  term_game;

Sdl sdl;
SDL_Cursor *empty_cursor = 0, *std_cursor = 0;

/* under SDL2 we create a window and render via SDL_Renderer/SDL_Texture */

/* timer */
int cur_time, last_time;

/* sdl surface */

#define LG_SURFACE_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888
#define LG_SURFACE_BPP 32

/* return full path of bitmap */
void get_full_bmp_path( char *full_path, const char *file_name )
{
    sprintf(full_path,  "%s/gfx/%s", get_gamedir(), file_name );
}

/*
    load a surface from file putting it in soft or hardware mem
*/
SDL_Surface* load_surf(const char *fname, int f)
{
    SDL_Surface *buf;
    SDL_Surface *new_sur;
    char path[ 512 ];

    get_full_bmp_path( path, fname );

    buf = SDL_LoadBMP( path );

    if ( buf == 0 ) {
        fprintf( stderr, "%s: %s\n", fname, SDL_GetError() );
        if ( f & SDL_NONFATAL )
            return 0;
        else
            exit( 1 );
    }
    new_sur = SDL_ConvertSurfaceFormat(buf, LG_SURFACE_PIXEL_FORMAT, 0);
    SDL_FreeSurface( buf );
    if ( new_sur == NULL ) {
        fprintf( stderr, "%s: %s\n", fname, SDL_GetError() );
        if ( f & SDL_NONFATAL )
            return 0;
        else
            exit( 1 );
    }
    SDL_SetColorKey( new_sur, SDL_TRUE, 0x0 );
    /* We rely on direct pixel reads (e.g. for color key detection), so keep RLE disabled. */
    SDL_SetSurfaceRLE( new_sur, SDL_FALSE );
    SDL_SetSurfaceBlendMode(new_sur, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceAlphaMod(new_sur, 255);
    return new_sur;
}

SDL_Surface* colorkey_to_alpha(SDL_Surface *surf, Uint32 color_key)
{
    if (!surf)
        return NULL;

    Uint8 r = 0, g = 0, b = 0;
    SDL_GetRGB(color_key, surf->format, &r, &g, &b);

    SDL_Surface *converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    if (!converted)
        return surf;
    if (converted != surf)
        SDL_FreeSurface(surf);
    surf = converted;

    color_key = SDL_MapRGB(surf->format, r, g, b);

    if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
    Uint8 *base = (Uint8 *)surf->pixels;
    for (int y = 0; y < surf->h; ++y) {
        Uint32 *row = (Uint32 *)(base + y * surf->pitch);
        for (int x = 0; x < surf->w; ++x) {
            Uint32 rgb = row[x] & 0x00FFFFFF;
            if (rgb == (color_key & 0x00FFFFFF))
                row[x] = rgb; /* alpha 0 */
            else
                row[x] = rgb | 0xFF000000; /* alpha 255 */
        }
    }
    if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

    SDL_SetColorKey(surf, SDL_FALSE, 0);
    SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceRLE(surf, SDL_FALSE);
    return surf;
}

/*
    create an RGBA surface using the engine's preferred pixel format
*/
SDL_Surface* create_surf(int w, int h, int f)
{
    SDL_Surface *sur;
    (void)f;
    sur = SDL_CreateRGBSurfaceWithFormat(0, w, h, LG_SURFACE_BPP, LG_SURFACE_PIXEL_FORMAT);
    if (sur == 0) {
        fprintf(stderr, "ERR: ssur_create: not enough memory to create surface...\n");
        exit(1);
    }
    SDL_SetColorKey(sur, SDL_TRUE, 0x0);
    SDL_SetSurfaceRLE( sur, SDL_FALSE );
    SDL_SetSurfaceBlendMode(sur, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceAlphaMod(sur, 255);
    return sur;
}

void free_surf( SDL_Surface **surf )
{
    if ( *surf ) {
        SDL_FreeSurface( *surf );
        *surf = 0;
    }
}

/*
    lock surface
*/
inline void lock_surf(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_LockSurface(sur);
}

/*
    unlock surface
*/
inline void unlock_surf(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_UnlockSurface(sur);
}

/*
    blit surface with destination DEST and source SOURCE using it's actual alpha and color key settings
*/
void blit_surf(void)
{
    if (!sdl.s.s || !sdl.d.s) return;
    SDL_BlendMode old_mode;
    Uint8 old_alpha;
    if (SDL_GetSurfaceBlendMode(sdl.s.s, &old_mode) != 0) old_mode = SDL_BLENDMODE_NONE;
    SDL_GetSurfaceAlphaMod(sdl.s.s, &old_alpha);
    SDL_BlendMode desired = (sdl.s.s->format && sdl.s.s->format->Amask) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;
    SDL_SetSurfaceBlendMode(sdl.s.s, desired);
    SDL_SetSurfaceAlphaMod(sdl.s.s, 255);
    SDL_BlitSurface(sdl.s.s, &sdl.s.r, sdl.d.s, &sdl.d.r);
    SDL_SetSurfaceAlphaMod(sdl.s.s, old_alpha);
    SDL_SetSurfaceBlendMode(sdl.s.s, old_mode);
}

/*
    do an alpha blit
*/
void alpha_blit_surf(int alpha)
{
    if (!sdl.s.s || !sdl.d.s) return;
    SDL_BlendMode old_mode;
    Uint8 old_alpha;
    if (SDL_GetSurfaceBlendMode(sdl.s.s, &old_mode) != 0) old_mode = SDL_BLENDMODE_NONE;
    SDL_GetSurfaceAlphaMod(sdl.s.s, &old_alpha);
    SDL_SetSurfaceBlendMode(sdl.s.s, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(sdl.s.s, (Uint8)alpha);
    SDL_BlitSurface(sdl.s.s, &sdl.s.r, sdl.d.s, &sdl.d.r);
    SDL_SetSurfaceAlphaMod(sdl.s.s, old_alpha);
    SDL_SetSurfaceBlendMode(sdl.s.s, old_mode);
}

/*
    fill surface with color c
*/
void fill_surf(int c)
{
    SDL_FillRect(sdl.d.s, &sdl.d.r, SDL_MapRGB(sdl.d.s->format, c >> 16, (c >> 8) & 0xFF, c & 0xFF));
}

/* set clipping rect */
void set_surf_clip( SDL_Surface *surf, int x, int y, int w, int h )
{
    SDL_Rect rect = { x, y, w, h };
    if ( w == h || h == 0 )
        SDL_SetClipRect( surf, NULL );
    else
        SDL_SetClipRect( surf, &rect );
}

/* set pixel */
Uint32 set_pixel( SDL_Surface *surf, int x, int y, int pixel )
{
    int pos = 0;

    if (x < 0 || y < 0 || x >= surf->w || y >= surf->h)
	    return pixel;

    pos = y * surf->pitch + x * surf->format->BytesPerPixel;
    memcpy( surf->pixels + pos, &pixel, surf->format->BytesPerPixel );
    return pixel;
}

/* get pixel */
Uint32 get_pixel( SDL_Surface *surf, int x, int y )
{
    int pos = 0;
    Uint32 pixel = 0;

    pos = y * surf->pitch + x * surf->format->BytesPerPixel;
    memcpy( &pixel, surf->pixels + pos, surf->format->BytesPerPixel );
    return pixel;
}

/* renderer-backed presentation helpers */
static void destroy_render_chain(void)
{
    if (sdl.texture) {
        SDL_DestroyTexture(sdl.texture);
        sdl.texture = NULL;
    }
    if (sdl.renderer) {
        SDL_DestroyRenderer(sdl.renderer);
        sdl.renderer = NULL;
    }
    if (sdl.screen) {
        SDL_FreeSurface(sdl.screen);
        sdl.screen = NULL;
    }
}

static int build_render_chain(int width, int height)
{
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, renderer_flags);
    if (!sdl.renderer)
        sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_SOFTWARE);
    if (!sdl.renderer) {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        destroy_render_chain();
        return 0;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(sdl.renderer, width, height);
#if SDL_VERSION_ATLEAST(2,0,5)
    SDL_RenderSetIntegerScale(sdl.renderer, SDL_FALSE);
#endif
    SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    sdl.texture = SDL_CreateTexture(sdl.renderer, LG_SURFACE_PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!sdl.texture) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
        destroy_render_chain();
        return 0;
    }

    sdl.screen = create_surf(width, height, 0);
    SDL_SetColorKey(sdl.screen, SDL_FALSE, 0);
    SDL_SetSurfaceBlendMode(sdl.screen, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceAlphaMod(sdl.screen, 255);

    sdl.rect_count = 0;
    return 1;
}

static void present_output(const SDL_Rect *rects, int count)
{
    if (!sdl.renderer || !sdl.texture || !sdl.screen)
        return;

    if (!rects || count <= 0) {
        SDL_UpdateTexture(sdl.texture, NULL, sdl.screen->pixels, sdl.screen->pitch);
    } else {
        const int pitch = sdl.screen->pitch;
        const int bpp = sdl.screen->format->BytesPerPixel;
        const Uint8 *base = (const Uint8 *)sdl.screen->pixels;
        for (int i = 0; i < count; ++i) {
            SDL_Rect r = rects[i];
            const void *start = base + r.y * pitch + r.x * bpp;
            SDL_UpdateTexture(sdl.texture, &r, start, pitch);
        }
    }

    SDL_RenderClear(sdl.renderer);
    SDL_RenderCopy(sdl.renderer, sdl.texture, NULL, NULL);
    SDL_RenderPresent(sdl.renderer);
}

/* sdl font */

/* return full font path */
void get_full_font_path( char *path, const char *file_name )
{
    strcpy( path, file_name );
/*    sprintf(path, "./gfx/fonts/%s", file_name ); */
}

/*
 * Loads glyphs into a font from file 'fname'.
 *
 * Font format description:
 *
 * Each font file is a pixmap comprised of a single row of subsequent glyphs
 * defining the appearence of the associated character code.
 *
 * The height of the font is implicitly defined by the height of the pixmap.
 *
 * Each glyph is defined by a pixmap as depicted in the following
 * example (where each letter represents the respective rgb-value):
 *
 * (1) -----> ........
 *            ........
 *            ..####..
 *            .#....#.
 *            .#....#.
 *            .#....#.
 *            .#....#.
 *            .#....#.
 *            .#.#..#.
 *            .#..#.#.
 *            ..####..
 *            ......#.
 * (2, 3) --> S.......
 *
 * (1): The top left pixel defines the transparency pixel. All pixels of
 * the same color as (1) are treated as transparent when the glyph is
 * rendered.
 * Note that (1) will only be checked for the *first* glyph in
 * the file and be applied to all other glyphs.
 *
 * (2): The bottom left pixel defines the starting column of a particular
 * glyph within the font pixmap. By scanning these pixels, the font loader
 * is able to determine the width of the glyph.
 * The color is either #FF00FF for a valid glyph, or #FF0000 for an invalid
 * glyph (in this case it will not be rendered,
 * regardless of what it contains otherwise).
 *
 * (3): For the very first glyph, (2) has a special meaning. It provides
 * some basic information about the font to be loaded.
 * The r-value specifies the starting code of the first glyph.
 * The g- and b-values are reserved and must be (0)
 *
 * The glyph-to-character-code-mapping is done by starting with the code
 * specified by (3). Then the bottom line is scanned: For every occurrence of
 * (2) the code will be incremented, and appropriate offset and width values
 * be calculated. The total count of characters will be determined by the
 * count of glyphs contained within the file.
 *
 * For display, (2) and (3) will be assumed to have color (1).
 */
void font_load_glyphs(Font *font, const char *fname)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define R_INDEX 1
#  define G_INDEX 2
#  define B_INDEX 3
#else
#  define R_INDEX 2
#  define G_INDEX 1
#  define B_INDEX 0
#endif
#define AT_RGB(pixel, idx) (((Uint8 *)&(pixel))+(idx))
    char    path[512];
    int     i;
    Uint32  transparency_key;
    Uint8   start_code, reserved1, reserved2;
    SDL_Surface *new_glyphs;

    get_full_font_path( path, fname );

    if ((new_glyphs = load_surf(path, 0)) == 0) {
        fprintf(stderr, tr("Cannot load new glyphs surface: %s\n"), SDL_GetError());
        exit(1);
    }
    /* use (1) as transparency key */
    transparency_key = get_pixel( new_glyphs, 0, 0 );

    /* evaluate (3) */
    SDL_GetRGB(get_pixel( new_glyphs, 0, new_glyphs->h - 1 ), new_glyphs->format, &start_code, &reserved1, &reserved2);
    // bail out early and consistently for invalid input to minimise the amount
    // of font files out of spec
    if (reserved1 || reserved2) {
        fprintf(stderr, tr("Invalid font file: %d, %d\n"), reserved1, reserved2);
        exit(1);
    }

    /* update font data */
    /* FIXME: don't blindly override, merge smartly */
    font->height = new_glyphs->h;
	
    /* cut prevalent glyph row at where to insert the new glyph row,
     * and insert it, thus overriding only the glyph range that is defined
     * by new_glyphs.
     */
    {
        /* total width of previous conserved glyphs */
        int pre_width = font->char_offset[start_code];
        /* total width of following conserved glyphs */
        int post_width = (font->pic ? font->pic->w : 0) - pre_width;
        unsigned code = start_code;
        int fixup;	/* amount of pixels following offsets have to be fixed up */
        SDL_Surface *dest;
        
        /* override widths and offsets of new glyphs */
        /* concurrently calculate width of following conserved glyphs */
        for (i = 0; i < new_glyphs->w; i++) {
            Uint32 pixel = 0;
            int valid_glyph;
            SDL_GetRGB(get_pixel(new_glyphs, i, new_glyphs->h - 1), new_glyphs->format, AT_RGB(pixel, R_INDEX), AT_RGB(pixel, G_INDEX), AT_RGB(pixel, B_INDEX));
            pixel &= 0xf8f8f8;	/* cope with RGB565 and other perversions */
            if (i != 0 && pixel != 0xf800f8 && pixel != 0xf80000) continue;

            if (code > 256) {
                fprintf(stderr, tr("font '%s' contains too many glyphs\n"), path);
                break;
            }

            valid_glyph = i == 0 || pixel != 0xf80000;
            set_pixel(new_glyphs, i, new_glyphs->h - 1, transparency_key);
            
            font->char_offset[code] = pre_width + i;
            font->keys[code] = valid_glyph;
            code++;
            
        }
        
        fixup = pre_width + i - font->char_offset[code];
        post_width -= font->char_offset[code] - font->char_offset[start_code];
        
        /* now the gory part:
         * 1. create a new surface large enough to hold conserved and new glyphs.
         * 2. blit the conserved previous part.
         * 3. blit the conserved following part.
         * 4. blit the new glyphs.
         */
            dest = create_surf(pre_width + new_glyphs->w + post_width,
            	   font->pic ? font->pic->h : new_glyphs->h, 0);
        if (!dest) {
            fprintf(stderr, tr("could not create font surface: %s\n"), SDL_GetError());
            exit(1);
        }

        if (pre_width > 0) {
            assert(font->pic);
            DEST(dest, 0, 0, pre_width, font->height);
            SOURCE(font->pic, 0, 0);
            blit_surf();
        }
        
        if (post_width > 0) {
            assert(font->pic);
            DEST(dest, font->char_offset[code] + fixup, 0, post_width, font->height);
            SOURCE(font->pic, font->char_offset[code], 0);
            blit_surf();
        }
        
        DEST(dest, font->char_offset[start_code], 0, new_glyphs->w, font->height);
        FULL_SOURCE(new_glyphs);
        blit_surf();
        
        /* replace old row with newly composed row */
        SDL_FreeSurface(font->pic);
        font->pic = dest;

        /* fix up offsets of successors */
        for (i = code; i < 256; i++)
            font->char_offset[code] += fixup;
        font->width += fixup;
    }
    
    SDL_SetColorKey( font->pic, SDL_TRUE, transparency_key );

    SDL_FreeSurface(new_glyphs);
#undef R_INDEX
#undef G_INDEX
#undef B_INDEX
#undef AT_RGB
}

/**
 * create a new font from font file 'fname'
 */
Font* load_font(const char *fname)
{
    Font *fnt = calloc(1, sizeof(Font));
    if (fnt == 0) {
        fprintf(stderr, tr("ERR: %s: not enough memory\n"), __FUNCTION__);
        exit(1);
    }
    
    fnt->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    fnt->color = 0x00FFFFFF;
    
    font_load_glyphs(fnt, fname);
    return fnt;
}

/*
    free memory
*/
void free_font(Font **fnt)
{
    if ( *fnt ) {
        if ((*fnt)->pic) SDL_FreeSurface((*fnt)->pic);
        free(*fnt);
        *fnt = 0;
    }
}

/*
    write something with transparency
*/
int write_text(Font *fnt, SDL_Surface *dest, int x, int y, const char *str, int alpha)
{
    int len = strlen(str);
    int pix_len = text_width(fnt, str);
    int px = x, py = y;
    int i;
    SDL_Surface *spf = sdl.screen;
    const int * const ofs = fnt->char_offset;
	
    /* alignment */
    if (fnt->align & ALIGN_X_CENTER)
        px -= pix_len >> 1;
    else if (fnt->align & ALIGN_X_RIGHT)
        px -= pix_len;
    if (fnt->align & ALIGN_Y_CENTER)
        py -= (fnt->height >> 1 ) + 1;
    else
        if (fnt->align & ALIGN_Y_BOTTOM)
            py -= fnt->height;

    fnt->last_x = MAXIMUM(px, 0);
    fnt->last_y = MAXIMUM(py, 0);
    fnt->last_width = MINIMUM(pix_len, spf->w - fnt->last_x);
    fnt->last_height = MINIMUM(fnt->height, spf->h - fnt->last_y);

    if (alpha != 0) {
        SDL_SetSurfaceBlendMode(fnt->pic, SDL_BLENDMODE_BLEND);
        SDL_SetSurfaceAlphaMod(fnt->pic, (Uint8)alpha);
    } else {
        SDL_SetSurfaceBlendMode(fnt->pic, SDL_BLENDMODE_NONE);
        SDL_SetSurfaceAlphaMod(fnt->pic, 255);
    }
    for (i = 0; i < len; i++) {
        unsigned c = (unsigned char)str[i];
        if (!fnt->keys[c]) c = '\177';
        {
            const int w = ofs[c+1] - ofs[c];
            DEST(dest, px, py, w, fnt->height);
            SOURCE(fnt->pic, ofs[c], 0);
            blit_surf();
            px += w;
        }
    }
    
    return 0;
}

/*
====================================================================
Write string to x, y and modify y so that it draws to the 
next line.
====================================================================
*/
void write_line( SDL_Surface *surf, Font *font, const char *str, int x, int *y )
{
    write_text( font, surf, x, *y, str, 255 );
    *y += font->height;
}

/*
    lock font surface
*/
inline void lock_font(Font *fnt)
{
    if (SDL_MUSTLOCK(fnt->pic))
        SDL_LockSurface(fnt->pic);
}

/*
    unlock font surface
*/
inline void unlock_font(Font *fnt)
{
    if (SDL_MUSTLOCK(fnt->pic))
        SDL_UnlockSurface(fnt->pic);
}
	
/*
    return last update region
*/
SDL_Rect last_write_rect(Font *fnt)
{
    SDL_Rect    rect={fnt->last_x, fnt->last_y, fnt->last_width, fnt->last_height};
    return rect;
}

inline int  char_width(Font *fnt, char c)
{
    unsigned i = (unsigned char)c;
    return fnt->char_offset[i + 1] - fnt->char_offset[i];
}

/*
    return the text width in pixels
*/
int text_width(Font *fnt, const char *str)
{
    unsigned int i;
    int pix_len = 0;
    for (i = strlen(str); i > 0; )
        pix_len += char_width(fnt, str[--i]);
    return pix_len;
}

/* sdl */

/*
    initialize sdl
*/
void init_sdl( int f )
{
    /* check flags: if SOUND is not enabled flag SDL_INIT_AUDIO musn't be set */
#ifndef WITH_SOUND
    if ( f & SDL_INIT_AUDIO )
        f = f & ~SDL_INIT_AUDIO;
#endif

    sdl.window = NULL;
    sdl.renderer = NULL;
    sdl.texture = NULL;
    sdl.screen = NULL;
    if (SDL_Init(f) < 0) {
        fprintf(stderr, "ERR: sdl_init: %s", SDL_GetError());
        exit(1);
    }
    atexit(quit_sdl);
    /* create empty cursor */
    empty_cursor = create_cursor( 16, 16, 8, 8,
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                " );
    std_cursor = SDL_GetCursor();
}

/*
    free screen
*/
void quit_sdl()
{
    destroy_render_chain();
    if (sdl.window) {
        SDL_DestroyWindow(sdl.window);
        sdl.window = NULL;
    }
    if ( empty_cursor ) SDL_FreeCursor( empty_cursor );
    if (sdl.vmodes) free(sdl.vmodes);
    SDL_Quit();
    printf("SDL finalized\n");
}

/** Get list of all video modes. Allocate @vmi and return number of 
 * entries. */
int get_video_modes( VideoModeInfo **vmi )
{
    int i, nmodes = 0;
    int num = SDL_GetNumDisplayModes(0);
    *vmi = NULL;
    if (num < 1) {
        VideoModeInfo stdvmi[2] = {
            { 800, 600, 32, 0 },
            { 800, 600, 32, 1 }
        };
        nmodes = 2;
        *vmi = calloc(nmodes, sizeof(VideoModeInfo));
        (*vmi)[0] = stdvmi[0];
        (*vmi)[1] = stdvmi[1];
        return nmodes;
    }
    nmodes = num * 2;
    *vmi = calloc(nmodes, sizeof(VideoModeInfo));
    for (i = 0; i < num; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0) {
            VideoModeInfo *info = &((*vmi)[i*2]);
            info->width = mode.w;
            info->height = mode.h;
            info->depth = 32;
            info->fullscreen = 0;
            (*vmi)[i*2+1] = *info;
            (*vmi)[i*2+1].fullscreen = 1;
        }
    }
    return nmodes;
}

/*
====================================================================
Switch to passed video mode.
====================================================================
*/
#define LG_WINDOW_TITLE "lgeneral"
int set_video_mode( int width, int height, int fullscreen )
{
#ifdef SDL_DEBUG
    SDL_PixelFormat	*fmt;
#endif
    /* if screen exists and matches, nothing to do */
    if (sdl.window) {
        int w = 0, h = 0;
        Uint32 flags = SDL_GetWindowFlags(sdl.window);
        int is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
        SDL_GetWindowSize(sdl.window, &w, &h);
        if (w == width && h == height && is_fullscreen == fullscreen)
            return 1;
        destroy_render_chain();
        SDL_DestroyWindow(sdl.window);
        sdl.window = NULL;
    }

    Uint32 win_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen) win_flags |= SDL_WINDOW_FULLSCREEN;
    sdl.window = SDL_CreateWindow(LG_WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, win_flags);
    if (!sdl.window) {
        fprintf(stderr, "%s\n", SDL_GetError());
        /* fallback to 800x600 */
        sdl.window = SDL_CreateWindow(LG_WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
        if (!sdl.window) {
            fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
            return 0;
        }
        width = 800;
        height = 600;
    }
    if (!build_render_chain(width, height)) {
        SDL_DestroyWindow(sdl.window);
        sdl.window = NULL;
        return 0;
    }

#ifdef SDL_DEBUG
    if (sdl.screen) {
        fmt = sdl.screen->format;
        printf("video mode format:\n");
        printf("Masks: R=%i, G=%i, B=%i\n", fmt->Rmask, fmt->Gmask, fmt->Bmask);
        printf("LShft: R=%i, G=%i, B=%i\n", fmt->Rshift, fmt->Gshift, fmt->Bshift);
        printf("RShft: R=%i, G=%i, B=%i\n", fmt->Rloss, fmt->Gloss, fmt->Bloss);
        printf("BBP: %i\n", fmt->BitsPerPixel);
        printf("-----\n");
    }
#endif
    return 1;
}

/*
    show hardware capabilities
*/
void hardware_cap()
{
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        printf("Display current mode: %dx%d, format: %u\n", mode.w, mode.h, mode.format);
    } else {
        printf("Unable to query display mode: %s\n", SDL_GetError());
    }
}

/*
    update rectangle (0,0,0,0)->fullscreen
*/
void refresh_screen(int x, int y, int w, int h)
{
    SDL_Rect rect;
    SDL_Rect *rect_ptr = NULL;
    int count = 0;

    if (w > 0 && h > 0) {
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        rect_ptr = &rect;
        count = 1;
    }

    present_output(rect_ptr, count);
    sdl.rect_count = 0;
}

/*
    draw all update regions
*/
void refresh_rects()
{
    present_output(sdl.rect_count ? sdl.rect : NULL, sdl.rect_count);
    sdl.rect_count = 0;
}

/*
    add update region/rect
*/
void add_refresh_region( int x, int y, int w, int h )
{
    if (!sdl.screen) return;
    if (sdl.rect_count == RECT_LIMIT) return;
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > sdl.screen->w)
        w = sdl.screen->w - x;
    if (y + h > sdl.screen->h)
        h = sdl.screen->h - y;
    if (w <= 0 || h <= 0)
        return;
    sdl.rect[sdl.rect_count].x = x;
    sdl.rect[sdl.rect_count].y = y;
    sdl.rect[sdl.rect_count].w = w;
    sdl.rect[sdl.rect_count].h = h;
    sdl.rect_count++;
}
void add_refresh_rect( SDL_Rect *rect )
{
    if ( rect )
        add_refresh_region( rect->x, rect->y, rect->w, rect->h );
}

/*
    fade screen to black
*/
void dim_screen(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (term_game) return;
    buffer = create_surf(sdl.screen->w, sdl.screen->h, 0);
    SDL_SetColorKey(buffer, SDL_FALSE, 0);
    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    for (i = 0; i <= trp; i += per_step) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        FULL_SOURCE(buffer);
        alpha_blit_surf(i);
        refresh_screen( 0, 0, 0, 0);
        SDL_Delay(delay);
    }
    if (trp == 255) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        refresh_screen( 0, 0, 0, 0);
    }
    SDL_FreeSurface(buffer);
#else
    refresh_screen( 0, 0, 0, 0);
#endif
}

/*
    undim screen
*/
void undim_screen(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (term_game) return;
    buffer = create_surf(sdl.screen->w, sdl.screen->h, 0);
    SDL_SetColorKey(buffer, SDL_FALSE, 0);
    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    for (i = trp; i >= 0; i -= per_step) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        FULL_SOURCE(buffer);
        alpha_blit_surf(i);
        refresh_screen( 0, 0, 0, 0);
        SDL_Delay(delay);
    }
    FULL_DEST(sdl.screen);
    FULL_SOURCE(buffer);
    blit_surf();
    refresh_screen( 0, 0, 0, 0);
    SDL_FreeSurface(buffer);
#else
    refresh_screen( 0, 0, 0, 0);
#endif
}

/*
    wait for a key
*/
int wait_for_key()
{
    /* wait for key */
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            term_game = 1;
            return 0;
        }
        if (event.type == SDL_KEYUP)
            return event.key.keysym.sym;
    }
}

/*
    wait for a key or mouse click
*/
void wait_for_click()
{
    /* wait for key or button */
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            term_game = 1;
            return;
        }
        if (event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONUP)
            return;
    }
}

/*
    lock surface
*/
inline void lock_screen()
{
    if (SDL_MUSTLOCK(sdl.screen))
        SDL_LockSurface(sdl.screen);
}

/*
    unlock surface
*/
inline void unlock_screen()
{
    if (SDL_MUSTLOCK(sdl.screen))
        SDL_UnlockSurface(sdl.screen);
}

/*
    flip hardware screens (double buffer)
*/
inline void flip_screen()
{
    present_output(NULL, 0);
    sdl.rect_count = 0;
}

/* cursor */

/* creates cursor */
SDL_Cursor* create_cursor( int width, int height, int hot_x, int hot_y, const char *source )
{
    unsigned char *mask = 0, *data = 0;
    SDL_Cursor *cursor = 0;
    int i, j, k;
    char data_byte, mask_byte;
    int pot;

    /* meaning of char from source:
        b : black, w: white, ' ':transparent */

    /* create mask&data */
    mask = malloc( width * height * sizeof ( char ) / 8 );
    data = malloc( width * height * sizeof ( char ) / 8 );

    k = 0;
    for (j = 0; j < width * height; j += 8, k++) {

        pot = 1;
        data_byte = mask_byte = 0;
        /* create byte */
        for (i = 7; i >= 0; i--) {

            switch ( source[j + i] ) {

                case 'b':
                    data_byte += pot;
                case 'w':
                    mask_byte += pot;
                    break;

            }
            pot *= 2;

        }
        /* add to mask */
        data[k] = data_byte;
        mask[k] = mask_byte;

    }

    /* create and return cursor */
    cursor = SDL_CreateCursor( data, mask, width, height, hot_x, hot_y );
    free( mask );
    free( data );
    return cursor;
}

/*
    get milliseconds since last call
*/
int get_time()
{
    int ms;
    cur_time = SDL_GetTicks();
    ms = cur_time - last_time;
    last_time = cur_time;
    if (ms == 0) {
        ms = 1;
        SDL_Delay(1);
    }
    return ms;
}

/*
    reset timer
*/
void reset_timer()
{
    last_time = SDL_GetTicks();
}

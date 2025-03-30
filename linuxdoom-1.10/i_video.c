// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
	rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// Had to dig up XShm.c for this one.
// It is in the libXext, but not in the XFree86 headers.

#include "w_wad.h"   // For W_CacheLumpName
#include "z_zone.h"  // For PU_STATIC

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include <SDL2/SDL.h>
#include "doomstat.h"
#include "doomdef.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#define POINTER_WARP_COUNTDOWN 1

int X_screen;

int X_width;
int X_height;



SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Color colors[256]; // Palette colors
byte gamepalette[256][3];
// MIT SHared Memory extension.
boolean doShm;

int X_shmeventtype;

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean grabMouse;
int doPointerWarp = POINTER_WARP_COUNTDOWN;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int multiply = 1;

//
//  Translates the key currently in X_event
//

int xlatekey(SDL_Event *event)
{
	switch (event->key.keysym.sym)
	{
	case SDLK_LEFT:
		return KEY_LEFTARROW;
	case SDLK_RIGHT:
		return KEY_RIGHTARROW;
	case SDLK_DOWN:
		return KEY_DOWNARROW;
	case SDLK_UP:
		return KEY_UPARROW;
	case SDLK_ESCAPE:
		return KEY_ESCAPE;
	case SDLK_RETURN:
		return KEY_ENTER;
	case SDLK_TAB:
		return KEY_TAB;
	case SDLK_F1:
		return KEY_F1;
	case SDLK_F2:
		return KEY_F2;
	case SDLK_F3:
		return KEY_F3;
	case SDLK_F4:
		return KEY_F4;
	case SDLK_F5:
		return KEY_F5;
	case SDLK_F6:
		return KEY_F6;
	case SDLK_F7:
		return KEY_F7;
	case SDLK_F8:
		return KEY_F8;
	case SDLK_F9:
		return KEY_F9;
	case SDLK_F10:
		return KEY_F10;
	case SDLK_F11:
		return KEY_F11;
	case SDLK_F12:
		return KEY_F12;
	case SDLK_BACKSPACE:
		return KEY_BACKSPACE;
	case SDLK_DELETE:
		return KEY_BACKSPACE;
	case SDLK_PAUSE:
		return KEY_PAUSE;

	default:
		if (event->key.keysym.sym >= SDLK_SPACE && event->key.keysym.sym <= SDLK_z)
			return (int)event->key.keysym.sym;
		break;
	}
	return 0;
}

void I_ShutdownGraphics(void)
{
	if (texture)
		SDL_DestroyTexture(texture);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	SDL_Quit();
}

//
// I_StartFrame
//
void I_StartFrame(void)
{
	// er?
}

void I_LoadPalette(void)
{
    byte* palettelump = W_CacheLumpName("PLAYPAL", PU_STATIC);
    memcpy(gamepalette, palettelump, 256 * 3);
}


static int lastmousex = 0;
static int lastmousey = 0;
boolean mousemoved = false;
boolean shmFinished;

void I_GetEvent(void)
{
    SDL_Event event;
    event_t doom_event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                doom_event.type = (event.type == SDL_KEYDOWN) ? ev_keydown : ev_keyup;

                switch (event.key.keysym.sym)
                {
                    // Movement (WASD + Arrow keys)
                    case SDLK_w:
                    case SDLK_UP:
                        doom_event.data1 = KEY_UPARROW;
                        break;

                    case SDLK_s:
                    case SDLK_DOWN:
                        doom_event.data1 = KEY_DOWNARROW;
                        break;

                    case SDLK_a:
                    case SDLK_LEFT:
                        doom_event.data1 = ',';
                        break;

                    case SDLK_d:
                    case SDLK_RIGHT:
                        doom_event.data1 = '.';
                        break;

                    // Actions
                    case SDLK_SPACE:
                        doom_event.data1 = ' '; // Fire weapon
                        break;

                    case SDLK_e:
                        doom_event.data1 = 'e';  // Open doors / use switches
                        break;

                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT:
                        doom_event.data1 = (0x80+0x12); // Run (speed modifier)
                        break;

                    // Automap
                    case SDLK_TAB:
                        doom_event.data1 = KEY_TAB; // Toggle automap
                        break;

                    // Menu / Pause
                    case SDLK_ESCAPE:
                        doom_event.data1 = KEY_ESCAPE; // Open menu
                        break;

                    // Weapon Selection (number keys)
                    case SDLK_1: doom_event.data1 = '1'; break;
                    case SDLK_2: doom_event.data1 = '2'; break;
                    case SDLK_3: doom_event.data1 = '3'; break;
                    case SDLK_4: doom_event.data1 = '4'; break;
                    case SDLK_5: doom_event.data1 = '5'; break;
                    case SDLK_6: doom_event.data1 = '6'; break;
                    case SDLK_7: doom_event.data1 = '7'; break;

                    default:
                        // Pass other keys directly
                        doom_event.data1 = event.key.keysym.sym;
                        break;
                }

                D_PostEvent(&doom_event);
                break;

            case SDL_MOUSEMOTION:
                doom_event.type = ev_mouse;
                doom_event.data1 = 0; // No buttons changed
                doom_event.data2 = event.motion.xrel << 5; // Sensitivity scale
                doom_event.data3 = -event.motion.yrel << 5; // Y-axis inverted
                D_PostEvent(&doom_event);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                doom_event.type = ev_mouse;
                doom_event.data1 =
                    (event.button.button == SDL_BUTTON_LEFT)   ? 1 :
                    (event.button.button == SDL_BUTTON_RIGHT)  ? 2 :
                    (event.button.button == SDL_BUTTON_MIDDLE) ? 4 : 0;

                if (event.type == SDL_MOUSEBUTTONUP)
                    doom_event.data1 = 0; // Clear buttons on release

                doom_event.data2 = 0;
                doom_event.data3 = 0;
                D_PostEvent(&doom_event);
                break;

            case SDL_QUIT:
                exit(0);
                break;
        }
    }
}

// Cursor
// createnullcursor(Display *display,
// 				 Window root)
// {
// 	Pixmap cursormask;
// 	XGCValues xgc;
// 	GC gc;
// 	XColor dummycolour;
// 	Cursor cursor;

// 	cursormask = XCreatePixmap(display, root, 1, 1, 1 /*depth*/);
// 	xgc.function = GXclear;
// 	gc = XCreateGC(display, cursormask, GCFunction, &xgc);
// 	XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
// 	dummycolour.pixel = 0;
// 	dummycolour.red = 0;
// 	dummycolour.flags = 04;
// 	cursor = XCreatePixmapCursor(display, cursormask, cursormask,
// 								 &dummycolour, &dummycolour, 0, 0);
// 	XFreePixmap(display, cursormask);
// 	XFreeGC(display, gc);
// 	return cursor;
// }

//
// I_StartTic
//
void I_StartTic(void)
{

    // Poll SDL events directly
    I_GetEvent();
    // If you need to recenter the mouse:
    if (grabMouse)
    {
        if (!--doPointerWarp)
        {
            SDL_WarpMouseInWindow(window, SCREENWIDTH / 2, SCREENHEIGHT / 2);
            doPointerWarp = POINTER_WARP_COUNTDOWN;
        }
    }
    mousemoved = false;
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
	// what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    static uint32_t argb_pixels[SCREENWIDTH * SCREENHEIGHT];
    byte* src = screens[0];

    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
    {
        byte colorindex = src[i];
        byte r = gamepalette[colorindex][0];
        byte g = gamepalette[colorindex][1];
        byte b = gamepalette[colorindex][2];

        argb_pixels[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }

    SDL_UpdateTexture(texture, NULL, argb_pixels, SCREENWIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// Palette stuff.
//
SDL_Color colors[256]; // Palette colors

void UploadNewPalette(byte *palette)
{
	int i;
	int c;

	for (i = 0; i < 256; i++)
	{
		c = gammatable[usegamma][*palette++];
		colors[i].r = c;

		c = gammatable[usegamma][*palette++];
		colors[i].g = c;

		c = gammatable[usegamma][*palette++];
		colors[i].b = c;

		colors[i].a = 255; // Opaque
	}

	// Apply the palette to the SDL texture if necessary
	SDL_SetPaletteColors(SDL_AllocPalette(256), colors, 0, 256);
}

//
// I_SetPalette
//
void I_SetPalette(byte *palette)
{
	UploadNewPalette(palette);
}

//
// This function is probably redundant,
//  if XShmDetach works properly.
// ddt never detached the XShm memory,
//  thus there might have been stale
//  handles accumulating.
//
// void grabsharedmemory(int size)
// {

// 	int key = ('d' << 24) | ('o' << 16) | ('o' << 8) | 'm';
// 	struct shmid_ds shminfo;
// 	int minsize = 320 * 200;
// 	int id;
// 	int rc;
// 	// UNUSED int done=0;
// 	int pollution = 5;

// 	// try to use what was here before
// 	do
// 	{
// 		id = shmget((key_t)key, minsize, 0777); // just get the id
// 		if (id != -1)
// 		{
// 			rc = shmctl(id, IPC_STAT, &shminfo); // get stats on it
// 			if (!rc)
// 			{
// 				if (shminfo.shm_nattch)
// 				{
// 					fprintf(stderr, "User %d appears to be running "
// 									"DOOM.  Is that wise?\n",
// 							shminfo.shm_cpid);
// 					key++;
// 				}
// 				else
// 				{
// 					if (getuid() == shminfo.shm_perm.cuid)
// 					{
// 						rc = shmctl(id, IPC_RMID, 0);
// 						if (!rc)
// 							fprintf(stderr,
// 									"Was able to kill my old shared memory\n");
// 						else
// 							I_Error("Was NOT able to kill my old shared memory");

// 						id = shmget((key_t)key, size, IPC_CREAT | 0777);
// 						if (id == -1)
// 							I_Error("Could not get shared memory");

// 						rc = shmctl(id, IPC_STAT, &shminfo);

// 						break;
// 					}
// 					if (size >= shminfo.shm_segsz)
// 					{
// 						fprintf(stderr,
// 								"will use %d's stale shared memory\n",
// 								shminfo.shm_cpid);
// 						break;
// 					}
// 					else
// 					{
// 						fprintf(stderr,
// 								"warning: can't use stale "
// 								"shared memory belonging to id %d, "
// 								"key=0x%x\n",
// 								shminfo.shm_cpid, key);
// 						key++;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				I_Error("could not get stats on key=%d", key);
// 			}
// 		}
// 		else
// 		{
// 			id = shmget((key_t)key, size, IPC_CREAT | 0777);
// 			if (id == -1)
// 			{
// 				extern int errno;
// 				fprintf(stderr, "errno=%d\n", errno);
// 				I_Error("Could not get any shared memory");
// 			}
// 			break;
// 		}
// 	} while (--pollution);

// 	if (!pollution)
// 	{
// 		I_Error("Sorry, system too polluted with stale "
// 				"shared memory segments.\n");
// 	}

// 	shminfo.shmid = id;

// 	// attach to the shared memory segment
// 	image->data = shminfo.shmaddr = shmat(id, 0, 0);

// 	fprintf(stderr, "shared memory id=%d, addr=0x%x\n", id,
// 			(int)(image->data));
// }

void I_InitGraphics(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("DOOM",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREENWIDTH, SCREENHEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        exit(1);
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);
    if (!texture)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        exit(1);
    }

    // Load the palette now:
    I_LoadPalette();
}

unsigned exptable[256];

void InitExpand(void)
{
	int i;

	for (i = 0; i < 256; i++)
		exptable[i] = i | (i << 8) | (i << 16) | (i << 24);
}

double exptable2[256 * 256];

void InitExpand2(void)
{
	int i;
	int j;
	// UNUSED unsigned	iexp, jexp;
	double *exp;
	union
	{
		double d;
		unsigned u[2];
	} pixel;

	printf("building exptable2...\n");
	exp = exptable2;
	for (i = 0; i < 256; i++)
	{
		pixel.u[0] = i | (i << 8) | (i << 16) | (i << 24);
		for (j = 0; j < 256; j++)
		{
			pixel.u[1] = j | (j << 8) | (j << 16) | (j << 24);
			*exp++ = pixel.d;
		}
	}
	printf("done.\n");
}

int inited;

void Expand4(unsigned *lineptr,
			 double *xline)
{
	double dpixel;
	unsigned x;
	unsigned y;
	unsigned fourpixels;
	unsigned step;
	double *exp;

	exp = exptable2;
	if (!inited)
	{
		inited = 1;
		InitExpand2();
	}

	step = 3 * SCREENWIDTH / 2;

	y = SCREENHEIGHT - 1;
	do
	{
		x = SCREENWIDTH;

		do
		{
			fourpixels = lineptr[0];

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff0000) >> 13));
			xline[0] = dpixel;
			xline[160] = dpixel;
			xline[320] = dpixel;
			xline[480] = dpixel;

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff) << 3));
			xline[1] = dpixel;
			xline[161] = dpixel;
			xline[321] = dpixel;
			xline[481] = dpixel;

			fourpixels = lineptr[1];

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff0000) >> 13));
			xline[2] = dpixel;
			xline[162] = dpixel;
			xline[322] = dpixel;
			xline[482] = dpixel;

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff) << 3));
			xline[3] = dpixel;
			xline[163] = dpixel;
			xline[323] = dpixel;
			xline[483] = dpixel;

			fourpixels = lineptr[2];

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff0000) >> 13));
			xline[4] = dpixel;
			xline[164] = dpixel;
			xline[324] = dpixel;
			xline[484] = dpixel;

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff) << 3));
			xline[5] = dpixel;
			xline[165] = dpixel;
			xline[325] = dpixel;
			xline[485] = dpixel;

			fourpixels = lineptr[3];

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff0000) >> 13));
			xline[6] = dpixel;
			xline[166] = dpixel;
			xline[326] = dpixel;
			xline[486] = dpixel;

			dpixel = *(double *)((int)exp + ((fourpixels & 0xffff) << 3));
			xline[7] = dpixel;
			xline[167] = dpixel;
			xline[327] = dpixel;
			xline[487] = dpixel;

			lineptr += 4;
			xline += 8;
		} while (x -= 16);
		xline += step;
	} while (y--);
}
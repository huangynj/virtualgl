/* Copyright (C)2004 Landmark Graphics
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

// FBX -- the fast Frame Buffer eXchange library
// This library is designed to facilitate transferring pixels to/from the framebuffer using fast
// 2D O/S-native methods that do not rely on OpenGL acceleration

#ifndef __FBX_H__
#define __FBX_H__

#define USESHM
#ifdef XDK
 #undef WIN32
#endif

#ifdef WIN32
 #include <windows.h>
 typedef HDC fbx_gc;
 typedef HWND fbx_wh;
#else
 #include <X11/Xlib.h>
 #ifdef USESHM
 #ifdef XDK
 #include <X11/hclshm.h>
 #else
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #endif
 #include <X11/extensions/XShm.h>
 #endif
 #include <X11/extensions/Xdbe.h>
 #include <X11/Xutil.h>
 typedef GC fbx_gc;
 typedef struct {Display *dpy; Window win;} fbx_wh;
#endif

#define BMPPAD(pitch) ((pitch+(sizeof(int)-1))&(~(sizeof(int)-1)))

enum {FBX_RGB, FBX_RGBA, FBX_BGR, FBX_BGRA};  // pixel formats

typedef struct _fbx_struct
{
	int ps;  // pixel size in bytes
	int width, height, pitch;
	int bgr; // 1 = BGR/BGRA, 0=RGB/RGBA
	char *bits;
	unsigned long rmask, gmask, bmask;
	fbx_wh wh;
	int shm;

	#ifdef WIN32
	HDC hmdc;  HBITMAP hdib;
	#else
	#ifdef USESHM
	XShmSegmentInfo shminfo;  int xattach;
	#endif
	GC xgc;
	XImage *xi;
	XdbeBackBuffer bb;
	#endif
} fbx_struct;

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////
// All of these methods return -1 on failure or 0 on success.
/////////////////////////////////////////////////////////////

/*
  fbx_init
  (fbx_struct *s, fbx_wh wh, int width, int height, int useshm)

  s = Address of fbx_struct (must be pre-allocated by user)
  wh = Handle to the window that you wish to read from or write to.  On Windows, this is the same
       as the HWND.  On Unix, this is a struct {Display *dpy, Window win} that describes the X11
       display and window you wish to use
  width = Width of buffer (in pixels) that you wish to create.  0 = use width of window
  height = Height of buffer (in pixels) that you wish to create.  0 = use height of window
  useshm = Use MIT-SHM extension, if available (Unix only)

  NOTES:
  -- fbx_init() is idempotent.  If you call it multiple times, it will re-initialize the
     buffer only when it is necessary to do so (such as when the window size has changed.)
  -- On Windows, fbx_init() will return a buffer configured with the same pixel format as the
     screen, unless the screen depth is < 24 bits, in which case it will always return a 32-bit BGRA
     buffer.

  On return, fbx_init() fills in the following relevant information in the fbx_struct that you
  passed to it:
  s->ps = pixel size of buffer in bytes
  s->bgr = 1 if the buffer uses BGRA pixel ordering or 0 otherwise
  s->rmask = bitmask used to obtain red component from a pixel
  s->gmask = bitmask used to obtain green component from a pixel
  s->bmask = bitmask used to obtain blue component from a pixel
  s->width, s->height = dimensions of the buffer
  s->bits = address of the start of the buffer
*/
int fbx_init(fbx_struct *s, fbx_wh wh, int width, int height, int useshm);

/*
  fbx_read
  (fbx_struct *s, int x, int y)

  This routine copies pixels from the framebuffer into the memory buffer specified by s

  s = Address of fbx_struct previously initialized by a call to fbx_init()
  x = Horizontal offset (from left of window client area) of rectangle to read
  y = Vertical offset (from top of window client area) of rectangle to read
  NOTE: width and height of rectangle are not adjustable without re-calling fbx_init()

  On return, s->bits contains a facsimile of the window's pixels
*/
int fbx_read(fbx_struct *s, int x, int y);

/*
  fbx_write
  (fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h)

  This routine copies pixels from the memory buffer specified by s to the framebuffer

  s = Address of fbx_struct previously initialized by a call to fbx_init()
      s->bits should contain the pixels you wish to blit
  bmpx = left offset of the region you wish to blit (relative to the memory buffer)
  bmpy = top offset of the region you wish to blit (relative to the memory buffer)
  winx = left offset of where you want the pixels to end up (relative to window client area)
  winy = top offset of where you want the pixels to end up (relative to window client area)
  w = width of region you wish to blit (0 = whole bitmap)
  h = height of region you wish to blit (0 = whole bitmap)
*/
int fbx_write (fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h);

/*
  fbx_awrite
  (fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h)

  Same as fbx_write, but asynchronous.  The write isn't guaranteed to complete
  until fbx_sync() is called.  On Windows, fbx_awrite is the same as fbx_write.
*/
#ifdef WIN32
#define fbx_awrite fbx_write
#else
int fbx_awrite (fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h);
#endif

/*
  fbx_sync
  (fbx_struct *s)

  Complete a previous asynchronous write.  On Windows, this does nothing.
*/
#ifdef WIN32
#define fbx_sync(s)
#else
int fbx_sync (fbx_struct *s);
#endif

/*
  fbx_term
  (fbx_struct *s)

  Free the memory buffers pointed to by structure s

  NOTE: this routine is idempotent.  It only frees stuff that needs freeing.
*/
int fbx_term(fbx_struct *s);

/*
  fbx_geterrmsg

  This returns a string containing the reason why the last command failed
*/
char *fbx_geterrmsg(void);

/*
  fbx_geterrline

  This returns the line (within fbx.c) of the last failure
*/
int fbx_geterrline(void);

#ifdef __cplusplus
}
#endif

#endif // __FBX_H__

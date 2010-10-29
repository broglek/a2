/* 
 * Copyright Rich West, October 2000.
 *
 * Graphical image viewer for ppm files (although easily adapted for rgb
 * and tiff files too).
 * 
 * This program requires either OpenGL or Mesa and the netpbm package.
 * To compile type: 
 * gcc -I./include -L/usr/X11R6/lib -L./lib/GL -L./lib/netpbm \
 * image.c -lGLU -lGL -lnetpbm -lXext -lX11 -lm -o image
 *
 */

#include "GL/glx.h"
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <netpbm/ppm.h>

Display *dpy;
Window window;

static void make_window (int width, int height, char *name, int border);

main(int argc,char *argv[]) {
  register pixel** pixarray;
  FILE *fp;
  int cols, rows;
  pixval maxval;
  
  register int x, y;
  unsigned char *buf;
  int border = 1;

  if (argc > 2 && argv[1][0] == '-' && argv[1][1] == 'n') {
    border = 0;
    argc--; argv++;
  }
  
  if (argc < 2) {
    fprintf (stderr, "Usage: %s [-n] filename.ppm\n", argv[0]);
    exit (1);
  } 
  
  if ((fp = fopen (argv[1],"r")) == NULL) {
    fprintf (stderr, "%s: Can't open input file %s.\n", argv[0], argv[1]);
    exit (1);
  }

  pixarray = ppm_readppm (fp, &cols, &rows, &maxval);

  buf = (unsigned char *)malloc (cols*rows*3);

  for (y = 0; y < rows; y++) {
    for (x = 0; x < cols; x++) {
      buf[(y*cols+x)*3+0] = PPM_GETR(pixarray[rows-y-1][x]);
      buf[(y*cols+x)*3+1] = PPM_GETG(pixarray[rows-y-1][x]);
      buf[(y*cols+x)*3+2] = PPM_GETB(pixarray[rows-y-1][x]);
    }
  }
  
  ppm_freearray (pixarray, rows);

  make_window (cols, rows, argv[1], border);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glMatrixMode (GL_PROJECTION);
  glOrtho (0, cols, 0, rows, -1, 1);
  glMatrixMode (GL_MODELVIEW);
  glRasterPos2i (0, 0);
  
  while (1) {
    XEvent ev;
    XNextEvent (dpy, &ev);
    switch (ev.type) {
    case Expose:
      glClearColor (0.5, 0.5, 0.5, 0.5);
      glClear (GL_COLOR_BUFFER_BIT);
      glDrawPixels (cols, rows, GL_RGB, GL_UNSIGNED_BYTE, buf);
      break;
      
    case KeyPress: {
      char buf2[100];
      int rv;
      KeySym ks;
      
      rv = XLookupString (&ev.xkey, buf2, sizeof(buf2), &ks, 0);
      switch (ks) {
      case XK_Escape:
	free (buf);
	exit(0);
      }
      break;
    }
    }
  }
}


static int attributeList[] = { GLX_RGBA, GLX_RED_SIZE, 1, None };

void noborder (Display *dpy, Window win) {
  struct {
    long flags;
    long functions;
    long decorations;
    long input_mode;
  } *hints;
  
  int fmt;
  unsigned long nitems, byaf;
  Atom type;
  Atom mwmhints = XInternAtom (dpy, "_MOTIF_WM_HINTS", False);

  XGetWindowProperty (dpy, win, mwmhints, 0, 4, False, mwmhints,
		      &type, &fmt, &nitems, &byaf, 
		      (unsigned char**)&hints);
  
  if (!hints) 
    hints = (void *)malloc (sizeof *hints);
  
  hints->decorations = 0;
  hints->flags |= 2;
  
  XChangeProperty (dpy, win, mwmhints, mwmhints, 32, PropModeReplace,
		   (unsigned char *)hints, 4);
  XFlush (dpy);
  free (hints);
}


static void make_window (int width, int height, char *name, int border) {
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;
  GLXContext cx;
  XSizeHints sizehints;
  
  dpy = XOpenDisplay (0);
  if (!(vi = glXChooseVisual (dpy, DefaultScreen(dpy), attributeList))) {
    printf ("Can't find requested visual.\n");
    exit (1);
  }
  cx = glXCreateContext (dpy, vi, 0, GL_TRUE);
  
  swa.colormap = XCreateColormap (dpy, RootWindow (dpy, vi->screen),
				  vi->visual, AllocNone);
  sizehints.flags = 0;
  
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
  window = XCreateWindow (dpy, RootWindow (dpy, vi->screen), 
			  0, 0, width, height,
			  0, vi->depth, InputOutput, vi->visual,
			  CWBorderPixel|CWColormap|CWEventMask, &swa);
  XMapWindow (dpy, window);
  XSetStandardProperties (dpy, window, name, name,
			  None, (void *)0, 0, &sizehints);
  
  if (!border) 
    noborder (dpy, window);
  
  glXMakeCurrent (dpy, window, cx);
}





















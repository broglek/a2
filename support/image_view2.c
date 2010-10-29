/* 
 * Copyright Rich West - October, 2001.
 *
 * Graphical image viewer for ppm files (although easily adapted for rgb
 * and tiff files too).
 *
 * This file is the same as image_view.c except it creates THREE 
 * identical windows and displays images in each.
 * 
 * This program requires either OpenGL or Mesa and the netpbm package.
 * To compile on Linux machines type: 
 * gcc -I./include -L/usr/X11R6/lib -L./lib/GL -L./lib/netpbm \
 * image_view2.c -lGLU -lGL -lnetpbm -lXext -lX11 -lm -o imview2
 *
 * To compile on other platforms or with the netpbm and GL packages in 
 * different locations, you need to use appropriate compile options.
 *
 * If Mesa or OpenGL is already installed, you need to find the location of 
 * the libraries and header files. Try a compile string like the following:
 *
 * gcc -L/usr/X11R6/lib image_view.c -lGLU -lGL -lnetpbm \
 * -lXext -lX11 -lm -o imview
 *
 * If this fails, you'll have to try some other configurations.
 */



#include <GL/glx.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <netpbm/ppm.h>
/* You might need to specify:
 * #include <ppm.h>
 *
 * instead of the line above, if using a distribution of netpbm
 * with the header files installed in /usr/include
 *
 * One day, I'll write a configure script :-)
 */

Display *dpy1, *dpy2, *dpy3;
Window window1, window2, window3;
GLXContext cx1, cx2, cx3;

static void make_window (int width, int height, char *name, int border,
			 Window *window, GLXContext *cx, Display **dpy);

main(int argc,char *argv[]) {
  register pixel** pixarray;
  FILE *fp;
  int cols, rows;
  pixval maxval;
  
  register int x, y;
  unsigned char *buf=NULL;
  int border = 1;
  int frame_count = 1;

  char s[80];

  make_window (160, 120, "Uncompressed Data Viewer1", 1, 
	       &window1, &cx1, &dpy1);
  make_window (160, 120, "Uncompressed Data Viewer2", 1, 
	       &window2, &cx2, &dpy2);
  make_window (160, 120, "Uncompressed Data Viewer3", 1, 
  &window3, &cx3, &dpy3); 

  glXMakeCurrent (dpy1, window1, cx1);

  glMatrixMode (GL_PROJECTION);
  glOrtho (0, cols, 0, rows, -1, 1);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glMatrixMode (GL_MODELVIEW);
  glRasterPos2i (0, 0);

  /* !!!!!!!!!!!! Atention CS552 !!!!!!!!!!!! 
   *
   * You need to modify the few lines below. In your producer/worker threads, 
   * you should read each image file into an array in memory, 
   * using something like:
   *
   * pixarray = ppm_readppm (fp, &cols, &rows, &maxval);
   *
   * where 'fp' is a file pointer.
   *
   * When a worker places an image in a slot all it really needs to
   * do is to have a pointer in the next free slot reference the array
   * in memory holding the next image.
   *
   * The dispatcher thread will obtain an image from a slot, when necessary,
   * by dereferencing the image pointer within the corresponding slot.
   *
   * The disptacher should write each image to a socket.
   *
   * Using the double 'for loop' shown below, the client threads can each
   * assign the RGB components of the image array (in this case 'pixarray')
   * to another array (in this case 'buf'), which stores the image in a form
   * suitable for being displayed.
   *    
   */


  while (frame_count <= 100) {
    sprintf (s, "./images/sw%d.ppm",
	     frame_count);
    frame_count++;
    if ((fp = fopen (s,"r")) == NULL) {
      fprintf (stderr, "%s: Can't open input file:\n %s.\n", s);
      exit (1);
    }
   
    /* Use the following line in the producer/worker threads.  */

    pixarray = ppm_readppm (fp, &cols, &rows, &maxval);
    fclose (fp);

    if (buf == NULL)
      buf = (unsigned char *)malloc (cols*rows*3);

    /* Use this double 'for loop' in the consumer/client threads.
     * You'll need to modify this code to read from a socket into 
     * a data structure called "pixarray" first.
     */

    for (y = 0; y < rows; y++) {
      for (x = 0; x < cols; x++) {
	buf[(y*cols+x)*3+0] = PPM_GETR(pixarray[rows-y-1][x]);
	buf[(y*cols+x)*3+1] = PPM_GETG(pixarray[rows-y-1][x]);
	buf[(y*cols+x)*3+2] = PPM_GETB(pixarray[rows-y-1][x]);
      }
    }

    ppm_freearray (pixarray, rows);

    if ((frame_count % 2) == 0)
      glXMakeCurrent (dpy2, window2, cx2);
    else
      if ((frame_count % 3) == 0)
	glXMakeCurrent (dpy3, window3, cx3);
    else
      glXMakeCurrent (dpy1, window1, cx1);

    glDrawPixels (cols, rows, GL_RGB, GL_UNSIGNED_BYTE, buf);
    glFlush ();
    
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


static void make_window (int width, int height, char *name, int border,
			 Window *window, GLXContext *cx, Display **dpy) {
  XVisualInfo *vi;
  XSetWindowAttributes swa;
  XSizeHints sizehints;
  
  *dpy = XOpenDisplay (0);
  if (!(vi = glXChooseVisual (*dpy, DefaultScreen(*dpy), attributeList))) {
    printf ("Can't find requested visual.\n");
    exit (1);
  }
  *cx = glXCreateContext (*dpy, vi, 0, GL_TRUE);
  
  swa.colormap = XCreateColormap (*dpy, RootWindow (*dpy, vi->screen),
				  vi->visual, AllocNone);
  sizehints.flags = 0;
  
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
  *window = XCreateWindow (*dpy, RootWindow (*dpy, vi->screen), 
			  0, 0, width, height,
			  0, vi->depth, InputOutput, vi->visual,
			  CWBorderPixel|CWColormap|CWEventMask, &swa);
  XMapWindow (*dpy, *window);
  XSetStandardProperties (*dpy, *window, name, name,
			  None, (void *)0, 0, &sizehints);
  
  if (!border) 
    noborder (*dpy, *window);
  
}

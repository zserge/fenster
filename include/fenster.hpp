#pragma once

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#define _DEFAULT_SOURCE 1
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <time.h>
#endif

#include <stdint.h>
#include <stdlib.h>

struct fenster {
  const char *title;
  const int width;
  const int height;
  uint32_t *buf;
  int keys[256]; /* keys are mostly ASCII, but arrows are 17..20 */
  int mod;       /* mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8 */
  int x;
  int y;
  int mouse;
#if defined(__APPLE__)
  id wnd;
#elif defined(_WIN32)
  HWND hwnd;
#else
  Display *dpy;
  Window w;
  GC gc;
  XImage *img;
#endif
};

int fenster_open(struct fenster *f);
int fenster_loop(struct fenster *f);
void fenster_close(struct fenster *f);
void fenster_sleep(int64_t ms);
int64_t fenster_time(void);
#define fenster_pixel(f, x, y) ((f)->buf[((y) * (f)->width) + (x)])

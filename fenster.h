#ifndef FENSTER_H
#define FENSTER_H

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__EMSCRIPTEN__)
#include <SDL/SDL.h>
#include <emscripten.h>
#  ifndef EM_ASYNCIFY
// EM_ASYNCIFY defaults to 1 to keep existing code working as normal
// but you have to compile with `-sASYNCIFY`
#    define EM_ASYNCIFY 1
#  endif
#else
#define _DEFAULT_SOURCE 1
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
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
#elif defined(__EMSCRIPTEN__)
  SDL_Surface *screen;
#else
  Display *dpy;
  Window w;
  GC gc;
  XImage *img;
  Atom wmDeleteMessage;
#endif
};

#define KEY_BACKSPACE   8
#define KEY_DELETE    127
#define KEY_ESCAPE     27
#define KEY_PAGEDOWN    4
#define KEY_PAGEUP      3
#define KEY_HOME        2
#define KEY_END         5
#define KEY_INSERT     26
#define KEY_RETURN     10
#define KEY_TAB         9
#define KEY_UP         17
#define KEY_DOWN       18
#define KEY_LEFT       20
#define KEY_RIGHT      19

#define MASK_CTRL  0x1
#define MASK_SHIFT 0x2
#define MASK_ALT   0x4
#define MASK_SUPER 0x8

#ifndef FENSTER_API
#define FENSTER_API extern
#endif
FENSTER_API int fenster_open(struct fenster *f);
FENSTER_API int fenster_loop(struct fenster *f);
FENSTER_API void fenster_close(struct fenster *f);
FENSTER_API void fenster_sleep(int64_t ms);
FENSTER_API int64_t fenster_time(void);
#define fenster_pixel(f, x, y) ((f)->buf[((y) * (f)->width) + (x)])

#ifndef FENSTER_HEADER

#if defined(__EMSCRIPTEN__)

FENSTER_API int fenster_open(struct fenster *f) {
	SDL_Init(SDL_INIT_VIDEO);
	f->screen = SDL_SetVideoMode(f->width, f->height, 32, SDL_SWSURFACE);
	emscripten_set_window_title(f->title);
	return 0;
}

// https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlkey.html
// clang-format off
static const int FENSTER_KEYCODES[] = {SDLK_BACKSPACE,8,SDLK_DELETE,127,SDLK_DOWN,18,SDLK_END,5,SDLK_ESCAPE,27,SDLK_HOME,2,SDLK_INSERT,26,SDLK_LEFT,20,SDLK_PAGEDOWN,4,SDLK_PAGEUP,3,SDLK_RETURN,10,SDLK_RIGHT,19,SDLK_TAB,9,SDLK_UP,17,SDLK_QUOTE,39,SDLK_BACKSLASH,92,SDLK_LEFTBRACKET,91,SDLK_RIGHTBRACKET,93,SDLK_COMMA,44,SDLK_EQUALS,61,SDLK_BACKQUOTE,96,SDLK_MINUS,45,SDLK_PERIOD,46,SDLK_SEMICOLON,59,SDLK_SLASH,47,SDLK_SPACE,32,SDLK_a,65,SDLK_b,66,SDLK_c,67,SDLK_d,68,SDLK_e,69,SDLK_f,70,SDLK_g,71,SDLK_h,72,SDLK_i,73,SDLK_j,74,SDLK_k,75,SDLK_l,76,SDLK_m,77,SDLK_n,78,SDLK_o,79,SDLK_p,80,SDLK_q,81,SDLK_r,82,SDLK_s,83,SDLK_t,84,SDLK_u,85,SDLK_v,86,SDLK_w,87,SDLK_x,88,SDLK_y,89,SDLK_z,90,SDLK_0,48,SDLK_1,49,SDLK_2,50,SDLK_3,51,SDLK_4,52,SDLK_5,53,SDLK_6,54,SDLK_7,55,SDLK_8,56,SDLK_9,57};
// clang-format on

FENSTER_API int fenster_loop(struct fenster *f) {

	// Unfortunately the screen is ABGR while the fenster buffer is ARGB
	if (SDL_MUSTLOCK(f->screen)) SDL_LockSurface(f->screen);
	for (int i = 0; i < f->width * f->height; i++) {
		uint32_t pixel = f->buf[i];
#if 0
		uint32_t r = (pixel >> 16) & 0xFF;
		uint32_t g = (pixel >> 8) & 0xFF;
		uint32_t b = (pixel >> 0) & 0xFF;
		*((Uint32*)f->screen->pixels + i) = SDL_MapRGBA(f->screen->format, r, g, b, 0);
#else
		pixel = (pixel & 0xFF00FF00) | ((pixel & 0xFF0000) >> 16) | ((pixel & 0xFF) << 16);
		*((Uint32*)f->screen->pixels + i) = pixel;
#endif
	}
	if (SDL_MUSTLOCK(f->screen)) SDL_UnlockSurface(f->screen);

	SDL_Flip(f->screen);

	SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
			case SDL_MOUSEMOTION: {
				f->x = event.motion.x;
				f->y = event.motion.y;
			} break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP: {
				f->x = event.button.x;
				f->y = event.button.y;
				if(event.button.button == SDL_BUTTON_LEFT)
					f->mouse = event.type == SDL_MOUSEBUTTONDOWN;
			} break;
			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				// event.key.keysym
				int mask = 0;
				switch(event.key.keysym.sym) {
					case SDLK_RSHIFT:
					case SDLK_LSHIFT: mask = 0x2; break;
					case SDLK_RCTRL:
					case SDLK_LCTRL: mask = 0x1; break;
					case SDLK_RALT:
					case SDLK_LALT: mask = 0x4; break;
					case SDLK_RSUPER:
					case SDLK_LSUPER: mask = 0x8; break;
				}
				if(mask) {
				  f->mod = (event.key.state == SDL_PRESSED) ? f->mod | mask : f->mod & ~mask;
				  break;
				}

				for (unsigned int i = 0; i < (sizeof FENSTER_KEYCODES / sizeof FENSTER_KEYCODES[0]); i += 2) {
					if (FENSTER_KEYCODES[i] == event.key.keysym.sym) {
					  f->keys[FENSTER_KEYCODES[i + 1]] = (event.key.state == SDL_PRESSED);
					  break;
				    }
			    }
			} break;
			default: break;
		}
	}

	return 0;
}

FENSTER_API void fenster_close(struct fenster *f) {
	(void)f;
	SDL_Quit();
}

#elif defined(__APPLE__)
#define msg(r, o, s) ((r(*)(id, SEL))objc_msgSend)(o, sel_getUid(s))
#define msg1(r, o, s, A, a)                                                    \
  ((r(*)(id, SEL, A))objc_msgSend)(o, sel_getUid(s), a)
#define msg2(r, o, s, A, a, B, b)                                              \
  ((r(*)(id, SEL, A, B))objc_msgSend)(o, sel_getUid(s), a, b)
#define msg3(r, o, s, A, a, B, b, C, c)                                        \
  ((r(*)(id, SEL, A, B, C))objc_msgSend)(o, sel_getUid(s), a, b, c)
#define msg4(r, o, s, A, a, B, b, C, c, D, d)                                  \
  ((r(*)(id, SEL, A, B, C, D))objc_msgSend)(o, sel_getUid(s), a, b, c, d)

#define cls(x) ((id)objc_getClass(x))

extern id const NSDefaultRunLoopMode;
extern id const NSApp;

static void fenster_draw_rect(id v, SEL s, CGRect r) {
  (void)r, (void)s;
  struct fenster *f = (struct fenster *)objc_getAssociatedObject(v, "fenster");
  CGContextRef context =
      msg(CGContextRef, msg(id, cls("NSGraphicsContext"), "currentContext"),
          "graphicsPort");
  CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider = CGDataProviderCreateWithData(
      NULL, f->buf, f->width * f->height * 4, NULL);
  CGImageRef img =
      CGImageCreate(f->width, f->height, 8, 32, f->width * 4, space,
                    kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,
                    provider, NULL, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(space);
  CGDataProviderRelease(provider);
  CGContextDrawImage(context, CGRectMake(0, 0, f->width, f->height), img);
  CGImageRelease(img);
}

static BOOL fenster_should_close(id v, SEL s, id w) {
  (void)v, (void)s, (void)w;
  msg1(void, NSApp, "terminate:", id, NSApp);
  return YES;
}

FENSTER_API int fenster_open(struct fenster *f) {
  msg(id, cls("NSApplication"), "sharedApplication");
  msg1(void, NSApp, "setActivationPolicy:", NSInteger, 0);
  f->wnd = msg4(id, msg(id, cls("NSWindow"), "alloc"),
                "initWithContentRect:styleMask:backing:defer:", CGRect,
                CGRectMake(0, 0, f->width, f->height), NSUInteger, 3,
                NSUInteger, 2, BOOL, NO);
  Class windelegate =
      objc_allocateClassPair((Class)cls("NSObject"), "FensterDelegate", 0);
  class_addMethod(windelegate, sel_getUid("windowShouldClose:"),
                  (IMP)fenster_should_close, "c@:@");
  objc_registerClassPair(windelegate);
  msg1(void, f->wnd, "setDelegate:", id,
       msg(id, msg(id, (id)windelegate, "alloc"), "init"));
  Class c = objc_allocateClassPair((Class)cls("NSView"), "FensterView", 0);
  class_addMethod(c, sel_getUid("drawRect:"), (IMP)fenster_draw_rect, "i@:@@");
  objc_registerClassPair(c);

  id v = msg(id, msg(id, (id)c, "alloc"), "init");
  msg1(void, f->wnd, "setContentView:", id, v);
  objc_setAssociatedObject(v, "fenster", (id)f, OBJC_ASSOCIATION_ASSIGN);

  id title = msg1(id, cls("NSString"), "stringWithUTF8String:", const char *,
                  f->title);
  msg1(void, f->wnd, "setTitle:", id, title);
  msg1(void, f->wnd, "makeKeyAndOrderFront:", id, nil);
  msg(void, f->wnd, "center");
  msg1(void, NSApp, "activateIgnoringOtherApps:", BOOL, YES);
  return 0;
}

FENSTER_API void fenster_close(struct fenster *f) {
  msg(void, f->wnd, "close");
}

// clang-format off
static const uint8_t FENSTER_KEYCODES[128] = {65,83,68,70,72,71,90,88,67,86,0,66,81,87,69,82,89,84,49,50,51,52,54,53,61,57,55,45,56,48,93,79,85,91,73,80,10,76,74,39,75,59,92,44,47,78,77,46,9,32,96,8,0,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,2,3,127,0,5,0,4,0,20,19,18,17,0};
// clang-format on
FENSTER_API int fenster_loop(struct fenster *f) {
  msg1(void, msg(id, f->wnd, "contentView"), "setNeedsDisplay:", BOOL, YES);
  id ev = msg4(id, NSApp,
               "nextEventMatchingMask:untilDate:inMode:dequeue:", NSUInteger,
               NSUIntegerMax, id, NULL, id, NSDefaultRunLoopMode, BOOL, YES);
  if (!ev)
    return 0;
  NSUInteger evtype = msg(NSUInteger, ev, "type");
  switch (evtype) {
  case 1: /* NSEventTypeMouseDown */
    f->mouse |= 1;
    break;
  case 2: /* NSEventTypeMouseUp*/
    f->mouse &= ~1;
    break;
  case 5:
  case 6: { /* NSEventTypeMouseMoved */
    CGPoint xy = msg(CGPoint, ev, "locationInWindow");
    f->x = (int)xy.x;
    f->y = (int)(f->height - xy.y);
    return 0;
  }
  case 10: /*NSEventTypeKeyDown*/
  case 11: /*NSEventTypeKeyUp:*/ {
    NSUInteger k = msg(NSUInteger, ev, "keyCode");
    f->keys[k < 127 ? FENSTER_KEYCODES[k] : 0] = evtype == 10;
    NSUInteger mod = msg(NSUInteger, ev, "modifierFlags") >> 17;
    f->mod = (mod & 0xc) | ((mod & 1) << 1) | ((mod >> 1) & 1);
    return 0;
  }
  }
  msg1(void, NSApp, "sendEvent:", id, ev);
  return 0;
}
#elif defined(_WIN32)

/* WS: Make sure the client area fits; center the window in the process */
static void FitWindow(struct fenster *f) {
    RECT rcClient, rwClient;
    POINT ptDiff;
    HWND hwndParent;
    RECT rcParent, rwParent;
    POINT ptPos;

    GetClientRect(f->hwnd, &rcClient);
    GetWindowRect(f->hwnd, &rwClient);
    ptDiff.x = (rwClient.right - rwClient.left) - rcClient.right;
    ptDiff.y = (rwClient.bottom - rwClient.top) - rcClient.bottom;

    hwndParent = GetParent(f->hwnd);
    if (NULL == hwndParent)
        hwndParent = GetDesktopWindow();

    GetWindowRect(hwndParent, &rwParent);
    GetClientRect(hwndParent, &rcParent);

    ptPos.x = rwParent.left + (rcParent.right - f->width) / 2;
    ptPos.y = rwParent.top + (rcParent.bottom - f->height) / 2;

    MoveWindow(f->hwnd, ptPos.x, ptPos.y, f->width + ptDiff.x, f->height + ptDiff.y, 0);
}

/* WS: TIL about `__argc` and `__argv`, which are global variables in Windows' C library */
#if !defined(NO_WINMAIN)
extern int main(int argc, char **argv);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	(void)hInstance, (void)hPrevInstance, (void)pCmdLine, (void)nCmdShow;
	return main(__argc, __argv);
}
#endif

// clang-format off
static const uint8_t FENSTER_KEYCODES[] = {0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,81,87,69,82,84,89,85,73,79,80,91,93,10,0,65,83,68,70,71,72,74,75,76,59,39,96,0,92,90,88,67,86,66,78,77,44,46,47,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,17,3,0,20,0,19,0,5,18,4,26,127};
// clang-format on
typedef struct BINFO{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[3];
}BINFO;
static LRESULT CALLBACK fenster_wndproc(HWND hwnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam) {
  struct fenster *f = (struct fenster *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch (msg) {
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP hbmp = CreateCompatibleBitmap(hdc, f->width, f->height);
    HBITMAP oldbmp = SelectObject(memdc, hbmp);
    BINFO bi = {{sizeof(bi), f->width, -f->height, 1, 32, BI_BITFIELDS}};
    bi.bmiColors[0].rgbRed = 0xff;
    bi.bmiColors[1].rgbGreen = 0xff;
    bi.bmiColors[2].rgbBlue = 0xff;
    SetDIBitsToDevice(memdc, 0, 0, f->width, f->height, 0, 0, 0, f->height,
                      f->buf, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    BitBlt(hdc, 0, 0, f->width, f->height, memdc, 0, 0, SRCCOPY);
    SelectObject(memdc, oldbmp);
    DeleteObject(hbmp);
    DeleteDC(memdc);
    EndPaint(hwnd, &ps);
  } break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
    f->mouse = (msg == WM_LBUTTONDOWN);
    break;
  case WM_MOUSEMOVE:
    f->y = HIWORD(lParam), f->x = LOWORD(lParam);
    break;
  case WM_KEYDOWN:
  case WM_KEYUP: {
    f->mod = ((GetKeyState(VK_CONTROL) & 0x8000) >> 15) |
             ((GetKeyState(VK_SHIFT) & 0x8000) >> 14) |
             ((GetKeyState(VK_MENU) & 0x8000) >> 13) |
             (((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12);
    f->keys[FENSTER_KEYCODES[HIWORD(lParam) & 0x1ff]] = !((lParam >> 31) & 1);
  } break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

FENSTER_API int fenster_open(struct fenster *f) {
  HINSTANCE hInstance = GetModuleHandle(NULL);
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc = fenster_wndproc;
  wc.hInstance = hInstance;
  wc.lpszClassName = f->title;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassEx(&wc);
  f->hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, f->title, f->title,
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						   CW_USEDEFAULT, CW_USEDEFAULT,
                           f->width, f->height, NULL, NULL, hInstance, NULL);

  if (f->hwnd == NULL)
    return -1;

  FitWindow(f);

  SetWindowLongPtr(f->hwnd, GWLP_USERDATA, (LONG_PTR)f);
  ShowWindow(f->hwnd, SW_NORMAL);
  UpdateWindow(f->hwnd);
  return 0;
}

FENSTER_API void fenster_close(struct fenster *f) { (void)f; }

FENSTER_API int fenster_loop(struct fenster *f) {
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT)
      return -1;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  InvalidateRect(f->hwnd, NULL, TRUE);
  return 0;
}
#else
// clang-format off
static int FENSTER_KEYCODES[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};
// clang-format on

FENSTER_API int fenster_open(struct fenster *f) {
  f->dpy = XOpenDisplay(NULL);
  int screen = DefaultScreen(f->dpy);
  
  int screen_width = DisplayWidth(f->dpy, screen);
  int screen_height = DisplayHeight(f->dpy, screen);
  int x = (screen_width - f->width) / 2;
  int y = (screen_height - f->height) / 2;

  f->w = XCreateSimpleWindow(f->dpy, RootWindow(f->dpy, screen), x, y, f->width,
                             f->height, 0, BlackPixel(f->dpy, screen),
                             WhitePixel(f->dpy, screen));

  XSizeHints hints = {
    .flags = PPosition | PSize | PMinSize | PMaxSize,
    .x = x,
    .y = y,
    .width = f->width,
    .height = f->height,
    .min_width = f->width,
    .max_width = f->width,
    .min_height = f->height,
    .max_height = f->height,
  };

  XSetWMNormalHints(f->dpy, f->w, &hints);

  f->gc = XCreateGC(f->dpy, f->w, 0, 0);
  XSelectInput(f->dpy, f->w,
               ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                   ButtonReleaseMask | PointerMotionMask);
  XStoreName(f->dpy, f->w, f->title);
  XMapWindow(f->dpy, f->w);
  XSync(f->dpy, f->w);
  f->img = XCreateImage(f->dpy, DefaultVisual(f->dpy, 0), 24, ZPixmap, 0,
                        (char *)f->buf, f->width, f->height, 32, 0);

  f->wmDeleteMessage = XInternAtom(f->dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(f->dpy, f->w, &f->wmDeleteMessage, 1);

  return 0;
}
FENSTER_API void fenster_close(struct fenster *f) { XCloseDisplay(f->dpy); }
FENSTER_API int fenster_loop(struct fenster *f) {
  XEvent ev;
  XPutImage(f->dpy, f->w, f->gc, f->img, 0, 0, 0, 0, f->width, f->height);
  XFlush(f->dpy);
  while (XPending(f->dpy)) {
    XNextEvent(f->dpy, &ev);
    switch (ev.type) {
      case ButtonPress:
      case ButtonRelease:
        f->mouse = (ev.type == ButtonPress);
        break;
      case MotionNotify:
        f->x = ev.xmotion.x, f->y = ev.xmotion.y;
        break;
      case KeyPress:
      case KeyRelease: {
        int mask = 0;
        int k = XkbKeycodeToKeysym(f->dpy, ev.xkey.keycode, 0, 0);
        switch(k) {
          case XK_Control_L:
          case XK_Control_R: mask = 0x1; break;
          case XK_Shift_L:
          case XK_Shift_R: mask = 0x2; break;
          case XK_Alt_L:
          case XK_Alt_R: mask = 0x4; break;
          case XK_Super_L:
          case XK_Super_R: mask = 0x8; break;
        }
        if(mask) {
          f->mod = (ev.type == KeyPress) ? f->mod | mask : f->mod & ~mask;
        } else {
          for (unsigned int i = 0; i < 124; i += 2) {
            if (FENSTER_KEYCODES[i] == k) {
              f->keys[FENSTER_KEYCODES[i + 1]] = (ev.type == KeyPress);
              break;
            }
          }
        }
      } break;
      case ClientMessage: { 
        if(ev.xclient.data.l[0] == (long int)f->wmDeleteMessage) {
          return -1;
        }
      } break;
    }
  }
  return 0;
}
#endif

#ifdef _WIN32
FENSTER_API void fenster_sleep(int64_t ms) { Sleep(ms); }
FENSTER_API int64_t fenster_time() {
  LARGE_INTEGER freq, count;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&count);
  return (int64_t)(count.QuadPart * 1000.0 / freq.QuadPart);
}
#elif defined(__EMSCRIPTEN__)
FENSTER_API void fenster_sleep(int64_t ms) {
	SDL_Delay((uint32_t)ms);
}

FENSTER_API int64_t fenster_time(void) {
	return (int64_t)SDL_GetTicks();
}
#else
FENSTER_API void fenster_sleep(int64_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}
FENSTER_API int64_t fenster_time(void) {
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  return time.tv_sec * 1000 + (time.tv_nsec / 1000000);
}
#endif

#ifdef __cplusplus
class Fenster {
  struct fenster f;
  int64_t now;

public:
  Fenster(const int w, const int h, const char *title)
      : f{.title = title, .width = w, .height = h} {
    this->f.buf = new uint32_t[w * h];
    this->now = fenster_time();
    fenster_open(&this->f);
  }
  ~Fenster() {
    fenster_close(&this->f);
    delete[] this->f.buf;
  }
  bool loop(const int fps) {
    int64_t t = fenster_time();
    if (t - this->now < 1000 / fps) {
      fenster_sleep(t - now);
    }
    this->now = t;
    return fenster_loop(&this->f) == 0;
  }
  inline uint32_t &px(const int x, const int y) {
    return fenster_pixel(&this->f, x, y);
  }
  bool key(int c) { return c >= 0 && c < 128 ? this->f.keys[c] : false; }
  int x() { return this->f.x; }
  int y() { return this->f.y; }
  int mouse() { return this->f.mouse; }
  int mod() { return this->f.mod; }
};
#endif /* __cplusplus */

#endif /* !FENSTER_HEADER */
#endif /* FENSTER_H */

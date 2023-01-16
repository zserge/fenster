# Fenster

Fenster /ˈfɛnstɐ/ -- a German word for "window".

This library provides the most minimal and highly opinionated way to display a cross-platform 2D canvas. If you remember Borland BGI or drawing things in QBASIC or `INT 10h`- you know what I mean.

## What it does for you

* Single application window of given size with a title.
* Application lifecycle and system events are all handled automatically.
* Minimal 24-bit RGB framebuffer.
* Cross-platform keyboard events (keycodes).
* Cross-platform timers to have a stable FPS rate.
* It's a single header in plain C99 of ~300LOC with no memory allocations.
* Go bindings (`import "github.com/zserge/fenster"`, see [godoc](https://pkg.go.dev/github.com/zserge/fenster))
* Zig bindings (see [examples/minimal-zig](/examples/minimal-zig))

## What it might do for you in the next version

* Mouse events (at least left button click + XY)
* Audio playback (WinMM, CoreAudio, ALSA)

## What it will never do for you

* GUI widgets - use Qt or Gtk or WxWidgets.
* Complex drawing routines or OpenGL - use Sokol or MiniFB or Tigr.
* Low-latency audio - use PortAudio, RtAudio, libsoundio etc.

In other words, you get a single super tiny C file, with a very simple API and it allows you to build graphical apps (simple games, emulators) in a very low-level retrocomputing manner.

## Example

Here's how to draw white noise:

```c
// main.c
#include "fenster.h"
#define W 320
#define H 240
int main() {
  uint32_t buf[W * H];
  struct fenster f = { .title = "hello", .width = W, .height = H, .buf = buf };
  fenster_open(&f);
  while (fenster_loop(&f) == 0) {
    for (int i = 0; i < W; i++) {
      for (int j = 0; j < H; j++) {
        fenster_pixel(&f, i, j) = rand();
      }
    }
  }
  fenster_close(&f);
  return 0;
}
```

Compile it and run:

```
# Linux
cc main.c -lX11 -o main && ./main
# macOS
cc main.c -framework Cocoa -o main && ./main
# windows
cc main.c -lgdi32 -o main.exe && main.exe
```

That's it.

## API

```c
struct fenster {
  const char *title; /* window title */
  const int width; /* window width */
  const int height; /* window height */
  uint32_t *buf; /* window pixels, 24-bit RGB, row by row, pixel by pixel */
  int keys[256]; /* keys are mostly ASCII, but arrows are 17..20 */
  int mod;       /* mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8 */
};
```

* `int fenster_open(struct fenster *f)` - opens a new app window.
* `int fenster_loop(struct fenster *f)` - handles system events and refreshes the canvas. Returns negative values when app window is closed.
* `void fenster_close(struct fenster *f)` - closes the window and exists the graphical app.
* `void fenster_sleep(int ms)` - pauses for `ms` milliseconds.
* `int64_t fenster_time()` - returns current time in milliseconds.
* `fenster_pixel(f, x, y) = 0xRRGGBB` - set pixel color.
* `uint32_t px = fenster_pixel(f, x, y);` - get pixel color.

See [examples/drawing-c](/examples/drawing-c) for more old-school drawing primitives, but also feel free to experiment with your own graphical algorithms!

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.

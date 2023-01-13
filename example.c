#include "fenster.h"

#define W 320
#define H 240

/* ============================================================
 * A very minimal example of a Fenster app:
 * - Opens a window
 * - Starts a loop
 * - Changes pixel colours based on some "shader" formula
 * - Sleeps if needed to maintain a frame rate of 60 FPS
 * - Closes a window
 * ============================================================ */
static int demo_minimal_framebuffer() {
  uint32_t buf[W * H];
  struct fenster f = {
      .title = "hello",
      .width = W,
      .height = H,
      .buf = buf,
  };
  fenster_open(&f);
  uint32_t t = 0;
  int64_t now = fenster_time();
  while (fenster_loop(&f) == 0) {
    t++;
    for (int i = 0; i < 320; i++) {
      for (int j = 0; j < 240; j++) {
        /* White noise: */
        /* fenster_pixel(&f, i, j) = (rand() << 16) ^ (rand() << 8) ^ rand(); */

        /* Colourful and moving: */
        /* fenster_pixel(&f, i, j) = i * j * t; */

        /* Munching squares: */
        fenster_pixel(&f, i, j) = i ^ j ^ t;
      }
    }
    int64_t time = fenster_time();
    if (time - now < 1000 / 60) {
      fenster_sleep(time - now);
    }
    now = time;
  }
  fenster_close(&f);
  return 0;
}

/* ============================================================
 * An example of using raster graphics with Fenster
 * - Line: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * - Circle: https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 * - Rectangle: well, it's obvious
 * - Flood fill: very inefficient, using recursion
 * - Text: small 5x3 bitmap font is used to render glyps
 * ============================================================ */
static void fenster_line(struct fenster *f, int x0, int y0, int x1, int y1,
                         uint32_t c) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;
  for (;;) {
    fenster_pixel(f, x0, y0) = c;
    if (x0 == x1 && y0 == y1) {
      break;
    }
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

static void fenster_rect(struct fenster *f, int x, int y, int w, int h,
                         uint32_t c) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      fenster_pixel(f, x + col, y + row) = c;
    }
  }
}

static void fenster_circle(struct fenster *f, int x, int y, int r, uint32_t c) {
  for (int dy = -r; dy <= r; dy++) {
    for (int dx = -r; dx <= r; dx++) {
      if (dx * dx + dy * dy <= r * r) {
        fenster_pixel(f, x + dx, y + dy) = c;
      }
    }
  }
}

static void fenster_fill(struct fenster *f, int x, int y, uint32_t old,
                         uint32_t c) {
  if (x < 0 || y < 0 || x >= f->width || y >= f->height) {
    return;
  }
  if (fenster_pixel(f, x, y) == old) {
    fenster_pixel(f, x, y) = c;
    fenster_fill(f, x - 1, y, old, c);
    fenster_fill(f, x + 1, y, old, c);
    fenster_fill(f, x, y - 1, old, c);
    fenster_fill(f, x, y + 1, old, c);
  }
}

// clang-format off
static uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
// clang-format on
static void fenster_text(struct fenster *f, int x, int y, char *s, int scale,
                         uint32_t c) {
  while (*s) {
    char chr = *s++;
    if (chr > 32) {
      uint16_t bmp = font5x3[chr - 32];
      for (int dy = 0; dy < 5; dy++) {
        for (int dx = 0; dx < 3; dx++) {
          if (bmp >> (dy * 3 + dx) & 1) {
            fenster_rect(f, x + dx * scale, y + dy * scale, scale, scale, c);
          }
        }
      }
    }
    x = x + 4 * scale;
  }
}

static int demo_raster_graphics() {
  uint32_t buf[W * H];
  struct fenster f = {
      .title = "hello",
      .width = W,
      .height = H,
      .buf = buf,
  };
  fenster_open(&f);
  uint32_t t = 0;
  int64_t now = fenster_time();
  while (fenster_loop(&f) == 0) {
    t++;
    fenster_rect(&f, 0, 0, W, H, 0x00333333);
    fenster_rect(&f, W / 4, H / 2, W / 2, H / 3, 0x00ff0000);
    fenster_rect(&f, W / 2, H / 2 + H / 12, W / 6, H / 3 - H / 12, 0x00ffffff);
    fenster_circle(&f, W / 2 - W / 8, H / 2 + H / 6, W / 20, 0x00ffffff);
    fenster_line(&f, W / 4 - 25, H / 2, W / 2, H / 4, 0x0000ffff);
    fenster_line(&f, W - W / 4 + 25, H / 2, W / 2, H / 4, 0x0000ffff);
    fenster_line(&f, W - W / 4 + 25, H / 2, W / 4 - 25, H / 2, 0x0000ffff);
    fenster_fill(&f, W / 2, H / 3, 0x00333333, 0x00ff00ff);
    fenster_text(&f, 10, 10, "House", 8, 0x00ffffff);
    int64_t time = fenster_time();
    if (time - now < 1000 / 60) {
      fenster_sleep(time - now);
    }
    now = time;
  }
  fenster_close(&f);
  return 0;
}

/* ============================================================
 * Another small example demostrating keymaps/keycodes:
 * - On all platforms keys usually correspond to upper-case ASCII
 * - Enter code is 10, Tab is 9, Backspace is 8, Escape is 27
 * - Delete is 127, Space is 32
 * - Modifiers are: Ctrl=1, Shift=2, Ctrl+Shift=3
 *
 * This demo prints currently pressed keys with modifiers.
 * ============================================================ */
static int demo_keys() {
  uint32_t buf[W * H] = {0};
  struct fenster f = {
      .title = "Press any key...",
      .width = W,
      .height = H,
      .buf = buf,
  };
  fenster_open(&f);
  int64_t now = fenster_time();
  while (fenster_loop(&f) == 0) {
    int has_keys = 0;
    char s[32];
    char *p = s;
    for (int i = 0; i < 128; i++) {
      if (f.keys[i]) {
        has_keys = 1;
        *p++ = i;
      }
    }
    *p = '\0';
    fenster_rect(&f, 8, 8, W, 100, 0);
    fenster_text(&f, 8, 8, s, 4, 0xffffff);
    if (has_keys) {
      if (f.mod & 1) {
        fenster_text(&f, 8, 40, "Ctrl", 4, 0xffffff);
      }
      if (f.mod & 2) {
        fenster_text(&f, 8, 80, "Shift", 4, 0xffffff);
      }
    }
    if (f.keys[27]) {
      break;
    }
    int64_t time = fenster_time();
    if (time - now < 1000 / 60) {
      fenster_sleep(time - now);
    }
    now = time;
  }
  fenster_close(&f);
  return 0;
}

static int run() {
	(void) demo_minimal_framebuffer;
	(void) demo_raster_graphics;
	(void) demo_keys;

	/*return demo_minimal_framebuffer();*/
	return demo_raster_graphics();
  /*return demo_keys();*/
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine,
                   int nCmdShow) {
  (void)hInstance, (void)hPrevInstance, (void)pCmdLine, (void)nCmdShow;
  return run();
}
#else
int main() { return run(); }
#endif


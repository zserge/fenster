#include "fenster.h"

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
static int run() {
  Fenster f(W, H, "hello c++");
  int t = 0;
  while (f.loop(60)) {
    for (int i = 0; i < W; i++) {
      for (int j = 0; j < H; j++) {
        f.px(i, j) = i ^ j ^ t;
      }
    }
		if (f.key(0x1b)) {
			break;
		}
    t++;
  }
  return 0;
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

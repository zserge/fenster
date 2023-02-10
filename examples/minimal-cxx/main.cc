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

#include "fenster.h"
#include "fenster_audio.h"

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
  uint32_t buf[W * H];
  struct fenster f = {
      .title = "hello",
      .width = W,
      .height = H,
      .buf = buf,
  };
  struct fenster_audio fa = {0};
  fenster_open(&f);
  fenster_audio_open(&fa);
  uint32_t t = 0, u = 0;
  float audio[FENSTER_AUDIO_BUFSZ];
  int64_t now = fenster_time();
  while (fenster_loop(&f) == 0) {
    t++;
    int n = fenster_audio_available(&fa);
    if (n > 0) {
      for (int i = 0; i < n; i++) {
        u++;
        /*audio[i] = (rand() & 0xff)/256.f;*/
        int x = u * 80 / 441;
        audio[i] = ((((x >> 10) & 42) * x) & 0xff) / 256.f;
      }
      fenster_audio_write(&fa, audio, n);
    }
    if (f.keys[27])
      break;

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
  fenster_audio_close(&fa);
  fenster_close(&f);
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

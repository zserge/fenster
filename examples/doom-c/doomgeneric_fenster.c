#include "../../../fenster.h"
#include "doomgeneric.h"
#include "doomkeys.h"

struct fenster f = {
    .width = DOOMGENERIC_RESX, .height = DOOMGENERIC_RESY, .title = "doom"};
void DG_Init() {
  f.buf = DG_ScreenBuffer;
  fenster_open(&f);
}
void DG_DrawFrame() { fenster_loop(&f); }
void DG_SleepMs(uint32_t ms) { fenster_sleep(ms); }
uint32_t DG_GetTicksMs() { return fenster_time(); }
void DG_SetWindowTitle(const char *title) { (void)title; }

unsigned char toDoomKey(int k) {
  switch (k) {
  case '\n':
    return KEY_ENTER;
  case '\x1b':
    return KEY_ESCAPE;
  case '\x11':
    return KEY_UPARROW;
  case '\x12':
    return KEY_DOWNARROW;
  case '\x13':
    return KEY_RIGHTARROW;
  case '\x14':
    return KEY_LEFTARROW;
  case 'Z':
    return KEY_FIRE;
  case 'X':
    return KEY_RSHIFT;
  case ' ':
    return KEY_USE;
  }
  return tolower(k);
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
  static int old[128] = {0};
  for (int i = 0; i < 128; i++) {
    if ((f.keys[i] && !old[i]) || (!f.keys[i] && old[i])) {
      *pressed = old[i] = f.keys[i];
      *doomKey = toDoomKey(i);
      return 1;
    }
  }
  return 0;
}

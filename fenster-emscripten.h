// this is an emscripten-only implementation of fenster
// make sure to add -sASYNCIFY

#include <stdint.h>
#include <stdlib.h>
#include "emscripten.h"

//                        offsets
struct fenster {      // byte   32bit
  const char *title;  // 0      0
  const int width;    // 4      1
  const int height;   // 8      2
  uint32_t *buf;      // 12     3
  int keys[256];      // 16     4
  int mod;            // 1040   260
  int x;              // 1044   261
  int y;              // 1048   262
  int mouse;          // 1052   263
};

#define fenster_pixel(f, x, y) ((f)->buf[((y) * (f)->width) + (x)])
#define fenster_sleep emscripten_sleep

static bool running = true;

EM_JS(int, fenster_open, (struct fenster *f), {
  if (!Module.canvas) {
    Module.canvas = document.getElementById('canvas');
    if (!Module.canvas) {
      document.createElement("canvas");
      document.appendElement(Module.canvas);
    }
  }
  // the HEAP32 offset of fenster object
  const p32 = f/4;
  const width = Module.HEAP32[p32 + 1];
  const height = Module.HEAP32[p32 + 2];
  Module.canvas.width = width;
  Module.canvas.height = height;
  Module.ctx = canvas.getContext("2d");
  Module.screen = Module.ctx.getImageData(0, 0, width, height);

  Module.canvas.addEventListener('mousemove', e => {
    Module.HEAP32[p32 + 261] = e.offsetX;
    Module.HEAP32[p32 + 262] = e.offsetY;
  });

  Module.canvas.addEventListener('mousedown', e => {
    Module.HEAP32[p32 + 263] = e.which;
  });

  Module.canvas.addEventListener('mouseup', e => {
    Module.HEAP32[p32 + 263] = 0;
  });

  Module.canvas.addEventListener('keydown', e => {
    Module.HEAP32[p32 + 4 + e.keyCode] = 1;

    let mod = 0;
    if (e.ctrlKey) {
      mod+=1
    }
    if (e.shiftKey) {
      mod+=2
    }
    if (e.altKey) {
      mod+=4
    }
    if (e.metaKey) {
      mod+=8
    }
    Module.HEAP32[p32 + 260] = mod;
  });

  Module.canvas.addEventListener('keyup', e => {
    Module.HEAP32[p32 + 4 + e.keyCode] = 0;

    // TODO: not sure if I need to do this twice
    let mod = 0;
    if (e.ctrlKey) {
      mod+=1
    }
    if (e.shiftKey) {
      mod+=2
    }
    if (e.altKey) {
      mod+=4
    }
    if (e.metaKey) {
      mod+=8
    }
    Module.HEAP32[p32 + 260] = mod;
  });
});

EM_JS(void, emscripten_fenster_loop, (int width, int height, uint32_t* buf), {
  const byteSize = width * height * 4;
  const buffer = Module.HEAPU8.slice(buf, buf + byteSize);
  // set alpha to 100% and re-arrange RGBA
  let r,g,b = 0;
  for (let i=0;i<byteSize;i+=4) {
    r = buffer[i+2];
    g = buffer[i+1];
    b = buffer[i+0];
    buffer[i+0] = r;
    buffer[i+1] = g;
    buffer[i+2] = b;
    buffer[i+3] = 0xff;
  }
  Module.screen.data.set(buffer);
  Module.ctx.putImageData(Module.screen, 0, 0);
});

int fenster_loop(struct fenster *f) {
  if (running) {
    emscripten_fenster_loop(f->width, f->height, f->buf);
  }

  // this is hack to make it not pin the main-loop (~60fps)
  // better would be if we can wrap the loop with emscripten_set_main_loop
  // which also does not require ASYNCIFY
  emscripten_sleep(16);

  return running ? 0 : 1;
}

void fenster_close(struct fenster *f) {
  running = false;
}

EM_JS(int64_t, fenster_time, (void), {
  return (new Date()).time;
});

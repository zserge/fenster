package fenster

/*
#cgo linux LDFLAGS: -lX11
#cgo darwin LDFLAGS: -framework Cocoa
#cgo windows LDFLAGS: -static -lgdi32
#include "fenster.h"
*/
import "C"
import (
	"fmt"
	"runtime"
	"unsafe"
)

func init() {
	// Ensure that main.main is called from the main thread
	runtime.LockOSThread()
}

type Fenster interface {
	Open(w, h int, title string) error
	Loop() bool
	Close()
	Pixel(x int, y int) uint32
	SetPixel(x int, y int, rgb uint32)
	Key(byte) bool
	Mod() Mods
}

type Mods int

type fenster struct {
	f   C.struct_fenster
	buf []uint32
}

func New() Fenster { return &fenster{} }
func (f *fenster) Open(w, h int, title string) error {
	f.f.title = C.CString(title)
	f.f.width = C.int(w)
	f.f.height = C.int(h)
	f.f.buf = (*C.uint32_t)(C.malloc(C.size_t(w * h * 4)))
	f.buf = unsafe.Slice((*uint32)(f.f.buf), w*h)
	res := C.fenster_open(&f.f)
	if res != 0 {
		return fmt.Errorf("failed to open window: %d", res)
	}
	return nil
}
func (f *fenster) Loop() bool { return C.fenster_loop(&f.f) == 0 }
func (f *fenster) Close()     { C.fenster_close(&f.f) }

func (f *fenster) Key(code byte) bool { return f.f.keys[code] != 0 }
func (f *fenster) Mod() Mods          { return Mods(f.f.mod) }

func (f *fenster) Pixel(x, y int) uint32           { return f.buf[y*int(f.f.width)+x] }
func (f *fenster) SetPixel(x, y int, color uint32) { f.buf[y*int(f.f.width)+x] = color }

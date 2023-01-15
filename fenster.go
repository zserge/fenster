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
	"image"
	"image/color"
	"image/draw"
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
	Key(byte) bool
	Mod() Mods
	draw.Image
}

// RGB Color 00RRGGBB
type RGB uint32

func (c RGB) RGBA() (r uint32, g uint32, b uint32, a uint32) {
	r = uint32(c>>16) & 0xff
	r |= r << 8
	g = uint32(c>>8) & 0xff
	g |= g << 8
	b = uint32(c) & 0xff
	b |= b << 8
	a = 0xffff
	return r, g, b, a
}

var RGBModel = color.ModelFunc(rgbModel)

func rgbModel(c color.Color) color.Color {
	if _, ok := c.(RGB); ok {
		return c
	}
	r, g, b, _ := c.RGBA()
	return RGB((r>>8)<<16 | (g>>8)<<8 | (b >> 8))
}

type Mods int

type fenster struct {
	f   C.struct_fenster
	buf []uint32
}

func (f *fenster) ColorModel() color.Model { return RGBModel }
func (f *fenster) Bounds() image.Rectangle {
	return image.Rect(0, 0, int(f.f.width), int(f.f.height))
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

func (f *fenster) At(x, y int) color.Color { return RGB(f.buf[y*int(f.f.width)+x]) }
func (f *fenster) Set(x, y int, c color.Color) {
	f.buf[y*int(f.f.width)+x] = uint32(f.ColorModel().Convert(c).(RGB))
}

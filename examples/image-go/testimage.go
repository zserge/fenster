package main

import (
	"bytes"
	_ "embed"
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"log"
	"math/rand"
	"time"

	"github.com/zserge/fenster"
)

//go:embed Testbild.png
var testimage []byte

func randomMask(r image.Rectangle) image.Image {
	img := image.NewAlpha(r)
	for x := r.Min.X; x < r.Max.X; x++ {
		for y := r.Min.Y; y < r.Max.Y; y++ {
			if rand.Uint32()%64 == 0 {
				img.Set(x, y, color.Opaque)
			}
		}
	}
	return img
}

func main() {
	f, err := fenster.New(320, 240, "TV")
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	bg, err := png.Decode(bytes.NewBuffer(testimage))
	if err != nil {
		log.Fatal(err)
	}

	for f.Loop(time.Second / 24) {
		// If escape is pressed - exit
		if f.Key(27) {
			break
		}

		draw.Draw(f, f.Bounds(), bg, image.Point{}, draw.Src)
		snow := randomMask(f.Bounds())
		draw.Draw(f, f.Bounds(), snow, image.Point{}, draw.Over)
	}
}

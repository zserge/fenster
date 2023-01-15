//go:build example

package main

import (
	"image"
	"image/color/palette"
	"image/draw"
	"image/png"
	"log"
	"os"
	"time"

	"github.com/zserge/fenster"
)

func openImage(fname string) (image.Image, error) {
	fd, err := os.Open(fname)
	if err != nil {
		return nil, err
	}
	defer fd.Close()
	return png.Decode(fd)
}

func main() {
	f, err := fenster.New(320, 240, "Hello")
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	img, err := openImage("testdata/Testbild.png")
	if err != nil {
		log.Fatal(err)
	}

	// convert to paletted image
	pimg := image.NewPaletted(img.Bounds(), palette.Plan9)
	draw.Draw(pimg, pimg.Bounds(), img, image.Point{}, draw.Src)

	for f.Loop(time.Second / 24) {
		// If escape is pressed - exit
		if f.Key(27) {
			break
		}

		// rotate palette
		pimg.Palette = append(pimg.Palette[1:], pimg.Palette[0])
		draw.Draw(f, f.Bounds(), pimg, image.Point{}, draw.Src)
	}
}

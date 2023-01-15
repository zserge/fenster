//go:build example

package main

import (
	"math/rand"
	"time"

	"github.com/zserge/fenster"
)

func main() {
	f := fenster.New()
	f.Open(320, 240, "Hello")
	defer f.Close()
	lastFrame := time.Now()
	for f.Loop() {
		// If escape is pressed - exit
		if f.Key(27) {
			break
		}
		// Fill screen with random noise
		for i := 0; i < 240; i++ {
			for j := 0; j < 320; j++ {
				f.Set(j, i, fenster.RGB(rand.Uint32()))
			}
		}
		// Wait for FPS rate
		sleep := 16*time.Millisecond - time.Since(lastFrame)
		if sleep > 0 {
			time.Sleep(sleep)
		}
		lastFrame = time.Now()
	}
}

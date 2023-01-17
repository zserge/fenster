package main

import (
	"log"
	"math/rand"
	"time"

	"github.com/zserge/fenster"
)

func main() {
	f, err := fenster.New(320, 240, "Hello")
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	for f.Loop(time.Second / 60) {
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
	}
}

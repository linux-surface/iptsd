package processing

import (
	"math"
)

type StylusProcessor struct {
	xCache [5]int
	yCache [5]int
}

func (sp *StylusProcessor) Flush() {
	for i := 0; i < len(sp.xCache); i++ {
		sp.xCache[i] = -1
		sp.yCache[i] = -1
	}
}

func (sp *StylusProcessor) Smooth(x int, y int) (int, int) {
	sp.xCache[0] = sp.xCache[1]
	sp.xCache[1] = sp.xCache[2]
	sp.xCache[2] = sp.xCache[3]
	sp.xCache[3] = sp.xCache[4]
	sp.xCache[4] = x

	sp.yCache[0] = sp.yCache[1]
	sp.yCache[1] = sp.yCache[2]
	sp.yCache[2] = sp.yCache[3]
	sp.yCache[3] = sp.yCache[4]
	sp.yCache[4] = y

	sx := 0
	sy := 0

	j := 0

	for i := 0; i < len(sp.xCache); i++ {
		if sp.xCache[i] == -1 || sp.yCache[i] == -1 {
			j++
			continue
		}

		sx += sp.xCache[i]
		sy += sp.yCache[i]
	}

	sx /= len(sp.xCache) - j
	sy /= len(sp.yCache) - j

	return sx, sy
}

func (sp *StylusProcessor) Tilt(altitude int, azimuth int) (int, int) {
	if altitude <= 0 {
		return 0, 0
	}

	alt := float64(altitude) / 18000 * math.Pi
	azm := float64(azimuth) / 18000 * math.Pi

	sin_alt := math.Sin(alt)
	sin_azm := math.Sin(azm)

	cos_alt := math.Cos(alt)
	cos_azm := math.Cos(azm)

	atan_x := math.Atan2(cos_alt, sin_alt*cos_azm)
	atan_y := math.Atan2(cos_alt, sin_alt*sin_azm)

	tx := 9000 - (atan_x * 4500 / (math.Pi / 4))
	ty := (atan_y * 4500 / (math.Pi / 4)) - 9000

	return int(tx), int(ty)
}

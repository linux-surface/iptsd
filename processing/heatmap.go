package processing

type Heatmap struct {
	Width  int
	Height int

	InvertX bool
	InvertY bool

	Data []byte
}

type TouchPoint struct {
	X     int
	Y     int
	Index int
}

func (hm Heatmap) Average() float32 {
	value := float32(0)

	for i := 0; i < len(hm.Data); i++ {
		value += float32(hm.Data[i])
	}

	return value / float32(len(hm.Data))
}

func (hm Heatmap) Value(x int, y int) byte {
	pos := y*hm.Width + x

	if pos >= len(hm.Data) || pos < 0 {
		return 0
	}

	return hm.Data[pos]
}

func (hm Heatmap) ContactPoints(points []TouchPoint) int {
	threshold := hm.Average() + 1
	p := 0

	if len(points) == 0 {
		return 0
	}

	for x := 0; x < hm.Width; x++ {
		for y := 0; y < hm.Height; y++ {
			val := hm.Value(x, y)

			if float32(val) < threshold {
				continue
			}

			ax := Max(x-1, 0)
			bx := Min(x+1, hm.Width-1)

			ay := Max(y-1, 0)
			by := Min(y+1, hm.Height-1)

			found := true
			for i := ax; i <= bx; i++ {
				for j := ay; j <= by; j++ {
					v1 := int(val)
					v2 := int(hm.Value(i, j))

					if CompareTriple(v1, x, y, v2, i, j) {
						continue
					}

					found = false
					break
				}

				if !found {
					break
				}
			}

			if !found {
				continue
			}

			points[p] = TouchPoint{X: x, Y: y, Index: -1}
			p++

			if p == len(points) {
				return p
			}
		}
	}

	return p
}

func (hm Heatmap) Coords(points []TouchPoint, count int) {
	for i := 0; i < count; i++ {
		x := float32(points[i].X)
		y := float32(points[i].Y)

		val := float32(hm.Value(points[i].X, points[i].Y))

		x += float32(hm.Value(points[i].X+1, points[i].Y)) / val
		x -= float32(hm.Value(points[i].X-1, points[i].Y)) / val

		y += float32(hm.Value(points[i].X, points[i].Y+1)) / val
		y -= float32(hm.Value(points[i].X, points[i].Y-1)) / val

		if hm.InvertX {
			x = float32(hm.Width-1) - x
		}

		if hm.InvertY {
			y = float32(hm.Height-1) - y
		}

		// TODO: Don't use singletouch values for screensize?
		x = x / float32(hm.Width-1) * 32767
		y = y / float32(hm.Height-1) * 32767

		points[i].X = int(x)
		points[i].Y = int(y)
	}
}

func (hm Heatmap) GetInputs(points []TouchPoint) {
	count := hm.ContactPoints(points)

	hm.Coords(points, count)
	TrackFingers(points, count)

	for i := count; i < len(points); i++ {
		points[i].X = 0
		points[i].Y = 0
		points[i].Index = -1
	}
}

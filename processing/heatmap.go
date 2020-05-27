package processing

type Heatmap struct {
	Width  int
	Height int
	Data   []byte
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

func (hm Heatmap) ContactPoints(points []ContactPoint) int {
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

			points[p] = ContactPoint{X: x, Y: y}
			p++

			if p == len(points) {
				return p
			}
		}
	}

	return p
}

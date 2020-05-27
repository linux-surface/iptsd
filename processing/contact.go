package processing

type ContactPoint struct {
	X int
	Y int
}

// TODO: Implement parsing for contact area
type Contact struct {
	X int
	Y int
}

func (hm Heatmap) GetCoords(point ContactPoint) Contact {
	x := float32(point.X)
	y := float32(point.Y)

	val := float32(hm.Value(point.X, point.Y))

	x += float32(hm.Value(point.X+1, point.Y)) / val
	x -= float32(hm.Value(point.X-1, point.Y)) / val

	y += float32(hm.Value(point.X, point.Y+1)) / val
	y -= float32(hm.Value(point.X, point.Y-1)) / val

	// TODO: SB2 specific quirk, create generic implementation
	y = float32(hm.Height) - (y + 1)

	// TODO: Don't use singletouch values for screensize?
	x = x / float32(hm.Width) * 32767
	y = y / float32(hm.Height) * 32767

	return Contact{
		X: int(x),
		Y: int(y),
	}
}

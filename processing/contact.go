package processing

const (
	TOUCH_THRESHOLD = byte(30)
)

type Contact struct {
	X  int
	Y  int
	XX int
	YY int
	XY int
	W  int
}

func (hm *Heatmap) GetCluster(x int, y int, res *Contact) {
	v := hm.Value(x, y)

	if v < TOUCH_THRESHOLD {
		return
	}

	if hm.GetVisited(x, y) {
		return
	}

	res.Add(x, y, int(v))
	hm.SetVisited(x, y, true)

	hm.GetCluster(x+1, y, res)
	hm.GetCluster(x-1, y, res)
	hm.GetCluster(x, y+1, res)
	hm.GetCluster(x, y-1, res)
}

func (hm *Heatmap) Contacts(contacts []Contact) int {
	c := 0

	if len(contacts) == 0 {
		return 0
	}

	for i := 0; i < len(hm.Visited); i++ {
		hm.Visited[i] = false
	}

	for x := 0; x < hm.Width; x++ {
		for y := 0; y < hm.Height; y++ {
			if hm.Value(x, y) < TOUCH_THRESHOLD {
				continue
			}

			if hm.GetVisited(x, y) {
				continue
			}

			contacts[c] = Contact{}

			hm.GetCluster(x, y, &contacts[c])
			c += 1

			if c == len(contacts) {
				return c
			}
		}
	}

	return c
}

func (c *Contact) Add(x int, y int, w int) {
	c.X += w * x
	c.Y += w * y
	c.XX += w * x * x
	c.YY += w * y * y
	c.XY += w * x * y
	c.W += w
}

func (c *Contact) Join(d Contact) {
	c.X += d.X
	c.Y += d.Y
	c.XX += d.XX
	c.YY += d.YY
	c.XY += d.XY
	c.W += d.W
}

func (c *Contact) Mean() (float32, float32) {
	fX := float32(c.X)
	fY := float32(c.Y)
	fW := float32(c.W)

	return fX / fW, fY / fW
}

func (c *Contact) Cov() (float32, float32, float32) {
	fX := float32(c.X)
	fY := float32(c.Y)
	fXX := float32(c.XX)
	fYY := float32(c.YY)
	fXY := float32(c.XY)
	fW := float32(c.W)

	r1 := (fXX - (fX * fX / fW)) / fW
	r2 := (fYY - (fY * fY / fW)) / fW
	r3 := (fXY - (fX * fY / fW)) / fW

	return r1, r2, r3
}

func (c *Contact) Palm() bool {
	x, y, _ := c.Cov()
	return x > 1.5 || y > 1.5
}

package processing

import (
	"math"
)

const (
	TOUCH_THRESHOLD = byte(30)
)

type Contact struct {
	X      int
	Y      int
	XX     int
	YY     int
	XY     int
	W      int
	isPalm bool
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
			//fmt.Println(contacts[c].Mean())
			//fmt.Println(contacts[c].Cov())
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

func (c *Contact) Near(other Contact) bool {
	x1, y1 := c.Mean()
	x2, y2 := other.Mean()
	dx, dy := math.Abs(float64(x1-x2)), math.Abs(float64(y1-y2))

	vx, vy, _ := other.Cov()

	sx := 3.2*math.Sqrt(float64(vx)) + 10
	sy := 3.2*math.Sqrt(float64(vy)) + 10

	return dx <= sx && dy <= sy
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

func GetPalms(contacts []Contact, count int) {
	for i := 0; i < count; i++ {
		vx, vy, _ := contacts[i].Cov()
		if vx < 1.5 && vy < 1.5 {
			continue
		}
		contacts[i].isPalm = true
		for j := 0; j < count; j++ {
			if j == i || contacts[j].isPalm {
				continue
			}
			if contacts[j].Near(contacts[i]) {
				contacts[j].isPalm = true
			}
		}
	}
}

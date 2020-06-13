package processing

import (
	"math"
)

const (
	TOUCH_THRESHOLD = byte(10)
)

func Abs(x float32) float32 {
	return float32(math.Abs(float64(x)))
}

func Sqrt(x float32) float32 {
	return float32(math.Sqrt(float64(x)))
}

func Hypot(x float32, y float32) float32 {
	return float32(math.Hypot(float64(x), float64(y)))
}

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

func (c *Contact) Eigenvalues() (float32, float32) {
	vx, vy, cv := c.Cov()
	sqrtd := Sqrt(((vx-vy)*(vx-vy) + 4*cv*cv))

	r1 := (vx + vy + sqrtd) / 2
	r2 := (vx + vy - sqrtd) / 2

	return r1, r2
}

func (c *Contact) PCA(x float32, y float32) (float32, float32) {
	vx, vy, cv := c.Cov()

	l1, l2 := c.Eigenvalues()

	qx1 := vx + cv - l2
	qy1 := vy + cv - l2
	d1 := Hypot(qx1, qy1)
	qx1 /= d1
	qy1 /= d1

	qx2 := vx + cv - l1
	qy2 := vy + cv - l1
	d2 := Hypot(qx2, qy2)
	qx2 /= d2
	qy2 /= d2

	return qx1*x + qx2*y, qy1*x + qy2*y
}

func (c *Contact) Near(other Contact) bool {
	x1, y1 := c.Mean()
	x2, y2 := other.Mean()
	dx, dy := other.PCA(x1-x2, y1-y2)
	dx, dy = Abs(dx), Abs(dy)

	vx, vy := other.Eigenvalues()
	dx /= 3.2*Sqrt(vx) + 5
	dy /= 3.2*Sqrt(vy) + 5

	return dx*dx+dy*dy <= 1
}

func GetPalms(contacts []Contact, count int) {
	for i := 0; i < count; i++ {
		vx, vy := contacts[i].Eigenvalues()

		if vx < 1.0 && vy < 1.0 { // Regular touch
			continue
		}

		if vx < 3.0 && vx/vy > 1.8 && vx/vy < 3.4 { // Thumb
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

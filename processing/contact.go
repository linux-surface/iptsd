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

type Cluster struct {
	X    int
	Y    int
	XX   int
	YY   int
	XY   int
	W    int
	MaxV int
}

type Contact struct {
	X      float32 // center
	Y      float32
	Ev1    float32 // covariance matrix eigenvalues
	Ev2    float32
	qx1    float32 // covariance matrix eigenvectors
	qy1    float32
	qx2    float32
	qy2    float32
	MaxV   int
	isPalm bool
}

func (hm *Heatmap) GetCluster(x int, y int, res *Cluster) {
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

	var cluster Cluster

	for x := 0; x < hm.Width; x++ {
		for y := 0; y < hm.Height; y++ {
			if hm.Value(x, y) < TOUCH_THRESHOLD {
				continue
			}

			if hm.GetVisited(x, y) {
				continue
			}

			cluster = Cluster{}

			hm.GetCluster(x, y, &cluster)
			contacts[c].GetFromCluster(cluster)
			// ignore 0 variance contacts
			if contacts[c].Ev2 > 0 {
				c += 1
			}

			if c == len(contacts) {
				return c
			}
		}
	}

	return c
}

func (c *Cluster) Add(x int, y int, w int) {
	c.X += w * x
	c.Y += w * y
	c.XX += w * x * x
	c.YY += w * y * y
	c.XY += w * x * y
	c.W += w
	if c.MaxV < w {
		c.MaxV = w
	}
}

func (c *Cluster) Mean() (float32, float32) {
	fX := float32(c.X)
	fY := float32(c.Y)
	fW := float32(c.W)

	return fX / fW, fY / fW
}

func (c *Cluster) Cov() (float32, float32, float32) {
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

func (c *Cluster) Eigenvalues() (float32, float32) {
	vx, vy, cv := c.Cov()
	sqrtd := Sqrt(((vx-vy)*(vx-vy) + 4*cv*cv))

	r1 := (vx + vy + sqrtd) / 2
	r2 := (vx + vy - sqrtd) / 2

	return r1, r2
}

func (c *Contact) GetFromCluster(cluster Cluster) {
	c.X, c.Y = cluster.Mean()
	vx, vy, cv := cluster.Cov()

	sqrtd := Sqrt(((vx-vy)*(vx-vy) + 4*cv*cv))

	c.Ev1 = (vx + vy + sqrtd) / 2
	c.Ev2 = (vx + vy - sqrtd) / 2

	c.qx1 = vx + cv - c.Ev2
	c.qy1 = vy + cv - c.Ev2
	d1 := Hypot(c.qx1, c.qy1)
	c.qx1 /= d1
	c.qy1 /= d1

	c.qx2 = vx + cv - c.Ev1
	c.qy2 = vy + cv - c.Ev1
	d2 := Hypot(c.qx2, c.qy2)
	c.qx2 /= d2
	c.qy2 /= d2

	c.MaxV = cluster.MaxV

	c.isPalm = false
}

func (c *Contact) PCA(x float32, y float32) (float32, float32) {
	return c.qx1*x + c.qx2*y, c.qy1*x + c.qy2*y
}

func (c *Contact) Near(other Contact) bool {
	dx, dy := other.PCA(c.X-other.X, c.Y-other.Y)
	dx, dy = Abs(dx), Abs(dy)

	dx /= 3.2*Sqrt(other.Ev1) + 8
	dy /= 3.2*Sqrt(other.Ev2) + 8

	return dx*dx+dy*dy <= 1
}

func GetPalms(contacts []Contact, count int) {
	for i := 0; i < count; i++ {
		vx, vy, maxV := contacts[i].Ev1, contacts[i].Ev2, contacts[i].MaxV

		if vx < 0.6 || vx < 1.0 && maxV > 80 { // Regular touch
			continue
		}

		if (vx < 1.25 || vx < 3.5 && maxV > 90) && vx/vy > 1.8 { // Thumb
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

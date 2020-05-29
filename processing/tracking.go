package processing

import (
	"math"
)

const (
	MAX_INT = 1 << 32
)

var (
	last      []TouchPoint
	distances [][]int
	indices   [][]int
)

func PointDistance(a TouchPoint, b TouchPoint) int {
	dx := float64(a.X - b.X)
	dy := float64(a.Y - b.Y)

	return int(math.Sqrt(dx*dx + dy*dy))
}

func SavePoints(points []TouchPoint) {
	for i := 0; i < len(points); i++ {
		last[i] = points[i]
	}
}

func TrackFingers(points []TouchPoint, count int) {
	for i := 0; i < count; i++ {
		points[i].Index = i
	}

	/*
	 * On the first contact we have no data to compare with, so simply
	 * use the index in the array as the finger index.
	 */
	if last == nil {
		last = make([]TouchPoint, len(points))
		distances = make([][]int, len(points))
		indices = make([][]int, len(points))

		for i := 0; i < len(points); i++ {
			distances[i] = make([]int, len(points))
			indices[i] = make([]int, len(points))
		}

		SavePoints(points)
		return
	}

	for i := 0; i < len(points); i++ {
		for j := 0; j < len(points); j++ {
			indices[i][j] = j

			if points[i].Index == -1 || last[j].Index == -1 {
				distances[i][j] = MAX_INT - len(last) + j
				continue
			}

			distances[i][j] = PointDistance(points[i], last[j])
		}

		BubbleSort(indices[i], func(x, y int) bool {
			return distances[i][indices[i][x]] <
				distances[i][indices[i][y]]
		})
	}

	for i := 0; i < count; i++ {
		points[i].Index = last[indices[i][0]].Index
	}

	// Find duplicates
	for j := 1; j < len(points); j++ {
		duplicates := 0

		for i := 0; i < count; i++ {
			duplicated := false

			if points[i].Index == -1 {
				continue
			}

			for k := 0; k < count; k++ {
				if k == i {
					continue
				}

				if points[i].Index != points[k].Index {
					continue
				}

				if distances[i][j-1] < distances[k][j-1] {
					continue
				}

				duplicated = true
				break
			}

			if !duplicated {
				continue
			}

			points[i].Index = -2
			duplicates++
		}

		if duplicates == 0 {
			break
		}

		for i := 0; i < len(points); i++ {
			if points[i].Index != -2 {
				continue
			}

			points[i].Index = last[indices[i][j]].Index
			duplicates--

			if duplicates == 0 {
				break
			}
		}
	}

	// Assign new indices to those points who have index -1
	for i := 0; i < count; i++ {
		if points[i].Index != -1 {
			continue
		}

		for k := 0; k < len(points); k++ {
			found := true

			for j := 0; j < count; j++ {
				if i == j {
					continue
				}

				if points[j].Index != k {
					continue
				}

				found = false
				break
			}

			if !found {
				continue
			}

			points[i].Index = k
			break
		}
	}

	SavePoints(points)
}

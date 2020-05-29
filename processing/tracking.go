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

	/*
	 * For every current point, calculate the distance to all points from
	 * the last cycle. Then use these distances to create a sorted list
	 * of their indices, going from nearest to furthest away.
	 */
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

	/*
	 * Choose the index of the nearest point from the previous cycle
	 */
	for i := 0; i < count; i++ {
		points[i].Index = last[indices[i][0]].Index
	}

	/*
	 * The above selection will definitly lead to duplicates. For example,
	 * a new contact will always get the index 0, because that is the
	 * smallest distance that will be calculated (MAX_INT - len(last) + 0)
	 *
	 * To fix this we will iterate over the points, searching and fixing
	 * duplicates until every point has an unique index (or -1, because
	 * that is handled seperately).
	 */
	for j := 1; j < len(points); j++ {
		duplicates := 0

		for i := 0; i < count; i++ {
			duplicated := false

			if points[i].Index == -1 {
				continue
			}

			/*
			 * Point A is a duplicate of point B if they have the
			 * same index, and B is closer to the point from the
			 * last cycle with the same index.
			 */
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

			/*
			 * If we change the index now, the points that are
			 * checked after this one will think they are
			 * duplicates. We set the index to -2 and fix it up
			 * after all other points have been checked for this
			 * iteration.
			 */
			points[i].Index = -2
			duplicates++
		}

		/*
		 * If we haven't found any duplicates we don't need to
		 * continue searching for them.
		 */
		if duplicates == 0 {
			break
		}

		/*
		 * Update the index for all points with index -2 (duplicates)
		 *
		 * We started by using the index of the nearest point from the
		 * previous cycle. Since that resulted in a duplicate we use
		 * the next-nearest point (incremented the index). We will
		 * continue to do that until there are no duplicates anymore.
		 */
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

	/*
	 * If by now one of the found points still has the Index -1, it is a
	 * new point, so we need to find a free index for it to use.
	 *
	 * This is not really complicated but the code is not that simple.
	 * We iterate over all points to find the one with index -1. Then
	 * we go through every possible index to see if it is already used by
	 * other points. If we cannot find a point using the index we assign
	 * it and continue to the next point.
	 */
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

	/*
	 * Finally, we need to save the current list of points to use them in
	 * the next cycle of course.
	 *
	 * Since the points list is a cached array, we cannot just assign it,
	 * because then "points" and "last" would be identical. Instead we
	 * need to go through them and copy over every element.
	 */
	SavePoints(points)
}

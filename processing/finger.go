package processing

const STABILITY_THRESHOLD = 0.1

/*
 * Go doesn't really provide a way to sort a list using a custom
 * comparison, without also allocating a new list. The solution is
 * simple: Just make our own sort function.
 */
func BubbleSort(v []int, less func(i, j int) bool) {
	swapped := true

	for n := len(v); swapped; n-- {
		swapped = false

		for i := 1; i < n; i++ {
			if !less(i, i-1) {
				continue
			}

			tmp := v[i-1]
			v[i-1] = v[i]
			v[i] = tmp
			swapped = true
		}
	}
}

func (tp *TouchProcessor) FindDuplicates(count int, itr int) bool {
	duplicates := 0

	for i := 0; i < count; i++ {
		duplicated := false

		if tp.inputs[i].Index == -1 {
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

			if tp.inputs[i].Index != tp.inputs[k].Index {
				continue
			}

			if tp.distances[i][itr-1] < tp.distances[k][itr-1] {
				continue
			}

			duplicated = true
			break
		}

		if !duplicated {
			continue
		}

		/*
		 * If we change the index now, the inputs that are
		 * checked after this one will think they are
		 * duplicates. We set the index to -2 and fix it up
		 * after all other inputs have been checked for this
		 * iteration as well.
		 */
		tp.inputs[i].Index = -2
		duplicates++
	}

	/*
	 * If we haven't found any duplicates we don't need to
	 * continue searching for them.
	 */
	if duplicates == 0 {
		return false
	}

	/*
	 * Update the index for all points with index -2 (duplicates)
	 *
	 * We started by using the index of the nearest point from the
	 * previous cycle. Since that resulted in a duplicate we use
	 * the next-nearest point (incremented the index). We will
	 * continue to do that until there are no duplicates anymore.
	 */
	for i := 0; i < tp.MaxTouchPoints; i++ {
		if tp.inputs[i].Index != -2 {
			continue
		}

		last := tp.last[tp.indices[i][itr]]
		contact := tp.inputs[i].contact

		dev1 := tp.inputs[i].Ev1 - last.Ev1
		dev2 := tp.inputs[i].Ev2 - last.Ev2

		tp.inputs[i].Index = last.Index
		tp.inputs[i].IsPalm = contact.IsPalm || last.IsPalm
		tp.inputs[i].IsStable = dev1 < STABILITY_THRESHOLD && dev2 < STABILITY_THRESHOLD

		duplicates--

		if duplicates == 0 {
			break
		}
	}

	return true
}

func (tp *TouchProcessor) TrackFingers(count int) {
	/*
	 * For every current input, calculate the distance to all previous
	 * inputs. Then use these distances to create a sorted list
	 * of their indices, going from nearest to furthest.
	 */
	for i := 0; i < tp.MaxTouchPoints; i++ {
		for j := 0; j < tp.MaxTouchPoints; j++ {
			tp.indices[i][j] = j

			current := tp.inputs[i]
			last := tp.last[j]

			if current.Index == -1 || last.Index == -1 {
				tp.distances[i][j] = float64((1 << 30) + j)
				continue
			}

			tp.distances[i][j] = current.Dist(last)
		}

		BubbleSort(tp.indices[i], func(x, y int) bool {
			return tp.distances[i][tp.indices[i][x]] <
				tp.distances[i][tp.indices[i][y]]
		})
	}

	/*
	 * Choose the index of the closest previous input
	 */
	for i := 0; i < count; i++ {
		last := tp.last[tp.indices[i][0]]
		contact := tp.inputs[i].contact
		dev1 := tp.inputs[i].Ev1 - last.Ev1
		dev2 := tp.inputs[i].Ev2 - last.Ev2

		tp.inputs[i].Index = last.Index
		tp.inputs[i].IsPalm = contact.IsPalm || last.IsPalm
		tp.inputs[i].IsStable = dev1 < STABILITY_THRESHOLD && dev2 < STABILITY_THRESHOLD
	}

	/*
	 * The above selection will definitly lead to duplicates. For example,
	 * a new input will always get the index 0, because that is the
	 * smallest distance that will be calculated (2^30 + 0)
	 *
	 * To fix this we will iterate over the inputs, searching and fixing
	 * duplicates until every input has an unique index (or -1, which we
	 * will handle seperately).
	 */
	for j := 1; j < tp.MaxTouchPoints; j++ {
		if !tp.FindDuplicates(count, j) {
			break
		}
	}

	/*
	 * If by now one of the inputs still has the Index -1, it is a
	 * new one, so we need to find a free index for it to use.
	 *
	 * This is not really complicated but the code is not that simple.
	 * We iterate over all inputs to find the one with index -1. Then
	 * we go through every possible index to see if it is already used by
	 * other inputs. If we cannot find an input using the index we assign
	 * it and continue to the next one.
	 *
	 * We also need to assign unused slot ID's to all inputs that don't
	 * touch the screen. For the remaining inputs, we simply reuse the
	 * finger index as the slot ID. The slot ID needs to stay consistent,
	 * to prevent errors from libinput.
	 */
	for i := 0; i < tp.MaxTouchPoints; i++ {
		if tp.inputs[i].Index != -1 {
			tp.inputs[i].Slot = tp.inputs[i].Index
			continue
		}

		for k := 0; k < tp.MaxTouchPoints; k++ {
			if !tp.freeIndices[k] {
				continue
			}

			tp.freeIndices[k] = false
			tp.inputs[i].Slot = k
			if i < count {
				tp.inputs[i].Index = k
			}
			break
		}
	}
}

package processing

func Min(x int, y int) int {
	if x < y {
		return x
	}
	return y
}

func Max(x int, y int) int {
	if x > y {
		return x
	}
	return y
}

/*
 * This mimicks the behaviour of pythons (a1, a2, a3) >= (b1, b2, b3)
 */
func CompareTriple(a1, a2, a3 int, b1, b2, b3 int) bool {
	if b1 > a1 {
		return false
	}

	if b1 < a1 {
		return true
	}

	if b2 > a2 {
		return false
	}

	if b2 < a2 {
		return true
	}

	if b3 > a3 {
		return false
	}

	if b3 < a3 {
		return true
	}

	return b3 == a3
}

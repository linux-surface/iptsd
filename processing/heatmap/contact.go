package heatmap

func (hm Heatmap) Contacts(contacts []Contact) int {
	threshold := hm.Average() + 1
	c := 0

	if len(contacts) == 0 {
		return 0
	}

	for x := 0; x < hm.Width; x++ {
		for y := 0; y < hm.Height; y++ {
			if float32(hm.Value(x, y)) < threshold {
				continue
			}

			found := true

			for i := -1; i <= 1; i++ {
				for j := -1; j <= 1; j++ {
					if hm.Compare(x, y, x+i, y+j) {
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

			contacts[c] = Contact{
				X: x,
				Y: y,
			}
			c++

			if c == len(contacts) {
				return c
			}
		}
	}

	return c
}

func (hm Heatmap) Coords(contact Contact) (float32, float32) {
	x := float32(contact.X)
	y := float32(contact.Y)

	val := float32(hm.Value(contact.X, contact.Y))

	x += float32(hm.Value(contact.X+1, contact.Y)) / val
	x -= float32(hm.Value(contact.X-1, contact.Y)) / val

	y += float32(hm.Value(contact.X, contact.Y+1)) / val
	y -= float32(hm.Value(contact.X, contact.Y-1)) / val

	x = x / float32(hm.Width-1)
	y = y / float32(hm.Height-1)

	return x, y
}

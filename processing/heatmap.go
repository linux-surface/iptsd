package processing

type Heatmap struct {
	Width  int
	Height int
	Data   []byte
}

func (hm Heatmap) Average() float32 {
	value := float32(0)

	for i := 0; i < len(hm.Data); i++ {
		value += float32(hm.Data[i])
	}

	return value / float32(len(hm.Data))
}

func (hm *Heatmap) Value(x int, y int) byte {
	pos := y*hm.Width + x

	if x < 0 || x >= hm.Width || y < 0 || y >= hm.Height {
		return 0
	}

	return hm.Data[pos]
}

func (hm *Heatmap) Compare(x1 int, y1 int, x2 int, y2 int) bool {
	v1 := int(hm.Value(x1, y1))
	v2 := int(hm.Value(x2, y2))

	return CompareTriple(v1, x1, y1, v2, x2, y2)
}

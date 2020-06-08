package processing

type Heatmap struct {
	Width  int
	Height int
	Data   []byte
}

func (hm Heatmap) Value(x int, y int) byte {
	pos := y*hm.Width + x

	if pos >= len(hm.Data) || pos < 0 {
		return 0
	}

	return hm.Data[pos]
}

func (hm Heatmap) Compare(x1 int, y1 int, x2 int, y2 int) bool {
	v1 := int(hm.Value(x1, y1))
	v2 := int(hm.Value(x2, y2))

	return CompareTriple(v1, x1, y1, v2, x2, y2)
}

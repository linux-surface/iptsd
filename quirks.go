package main

const (
	IPTS_QUIRKS_NONE             = 0
	IPTS_QUIRKS_HEATMAP_INVERT_X = 1 << 0
	IPTS_QUIRKS_HEATMAP_INVERT_Y = 1 << 1
)

type IptsQuirks struct {
	data int
}

func (quirks *IptsQuirks) Add(q int) {
	quirks.data = quirks.data | q
}

func (quirks *IptsQuirks) AddForModel(info *IptsDeviceInfo, ven, pro, q int) {
	if info.Vendor != uint16(ven) || info.Product != uint16(pro) {
		return
	}

	quirks.Add(q)
}

func (quirks *IptsQuirks) Has(q int) bool {
	if (quirks.data & q) > 0 {
		return true
	}

	return false
}

func (quirks *IptsQuirks) Init(info *IptsDeviceInfo) {
	quirks.data = IPTS_QUIRKS_NONE

	// Surface Book 2 (13")
	quirks.AddForModel(info, 0x045E, 0x0021, IPTS_QUIRKS_HEATMAP_INVERT_Y)

	// Surface Pro 5
	quirks.AddForModel(info, 0x1B96, 0x001F, IPTS_QUIRKS_HEATMAP_INVERT_X)
	quirks.AddForModel(info, 0x1B96, 0x001F, IPTS_QUIRKS_HEATMAP_INVERT_Y)

	// Surface Pro 6
	quirks.AddForModel(info, 0x045E, 0x001F, IPTS_QUIRKS_HEATMAP_INVERT_X)
	quirks.AddForModel(info, 0x045E, 0x001F, IPTS_QUIRKS_HEATMAP_INVERT_Y)
}

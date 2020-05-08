package main

import (
	"bytes"
	"unsafe"
)

type IptsTouchHeatmap struct {
	Height  uint8
	Width   uint8
	Size    uint16
	Heatmap []byte
}

func IptsTouchHandleHeatmap(ipts *IPTS, heatmap IptsTouchHeatmap) {

}

func IptsTouchHandleInput(ipts *IPTS, buffer *bytes.Reader, frame IptsPayloadFrame) {
	size := uint32(0)
	heatmap := IptsTouchHeatmap{}

	for size < frame.Size {
		report := IptsReport{}

		IptsUtilsRead(buffer, &report)
		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			IptsUtilsRead(buffer, &heatmap.Height)
			IptsUtilsRead(buffer, &heatmap.Width)
			IptsUtilsSkip(buffer, uint32(6))
			break
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			heatmap.Size = report.Size
			heatmap.Heatmap = make([]byte, heatmap.Size)
			IptsUtilsRead(buffer, &heatmap.Heatmap)
			break
		default:
			// ignored
			IptsUtilsSkip(buffer, uint32(report.Size))
			break
		}
	}

	if heatmap.Size == 0 {
		return
	}

	mapsize := uint16(heatmap.Width) * uint16(heatmap.Height)
	if heatmap.Size != mapsize {
		return
	}

	IptsTouchHandleHeatmap(ipts, heatmap)
}

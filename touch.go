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

func IptsTouchHandleHeatmap(ipts *IptsContext, heatmap IptsTouchHeatmap) error {
	return nil
}

func IptsTouchParseHeatmapDim(buffer *bytes.Reader, heatmap *IptsTouchHeatmap) error {
	err := IptsUtilsRead(buffer, &heatmap.Height)
	if err != nil {
		return err
	}

	err = IptsUtilsRead(buffer, &heatmap.Width)
	if err != nil {
		return err
	}

	err = IptsUtilsSkip(buffer, uint32(6))
	if err != nil {
		return err
	}

	return nil
}

func IptsTouchHandleInput(ipts *IptsContext, buffer *bytes.Reader, frame IptsPayloadFrame) error {
	size := uint32(0)
	heatmap := IptsTouchHeatmap{}

	for size < frame.Size {
		report := IptsReport{}

		err := IptsUtilsRead(buffer, &report)
		if err != nil {
			return err
		}

		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			err = IptsTouchParseHeatmapDim(buffer, &heatmap)
			break
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			heatmap.Size = report.Size
			heatmap.Heatmap = make([]byte, heatmap.Size)
			err = IptsUtilsRead(buffer, &heatmap.Heatmap)
			break
		default:
			// ignored
			err = IptsUtilsSkip(buffer, uint32(report.Size))
			break
		}

		if err != nil {
			return nil
		}
	}

	if heatmap.Size == 0 {
		return nil
	}

	mapsize := uint16(heatmap.Width) * uint16(heatmap.Height)
	if heatmap.Size != mapsize {
		return nil
	}

	err := IptsTouchHandleHeatmap(ipts, heatmap)
	if err != nil {
		return err
	}

	return nil
}

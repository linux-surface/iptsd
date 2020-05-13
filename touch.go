package main

import (
	"unsafe"

	. "github.com/linux-surface/iptsd/protocol"
)

type IptsTouchHeatmap struct {
	Height  uint8
	Width   uint8
	Size    uint16
	Heatmap []byte
}

var (
	heatmapCache       IptsTouchHeatmap
	heatmapBufferCache map[uint16][]byte = make(map[uint16][]byte)
)

func IptsTouchHandleHeatmap(ipts *IptsContext, heatmap IptsTouchHeatmap) error {
	return nil
}

func IptsTouchHandleInput(ipts *IptsContext, frame IptsPayloadFrame) error {
	size := uint32(0)
	heatmap := IptsTouchHeatmap{}

	for size < frame.Size {
		report, err := ipts.Protocol.ReadReport()
		if err != nil {
			return err
		}

		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			heatmap.Height, err = ipts.Protocol.ReadByte()
			if err != nil {
				break
			}

			heatmap.Width, err = ipts.Protocol.ReadByte()
			if err != nil {
				break
			}

			err = ipts.Protocol.Skip(6)
			break
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			heatmap.Size = report.Size
			hmb, ok := heatmapBufferCache[heatmap.Size]
			if !ok {
				hmb = make([]byte, heatmap.Size)
				heatmapBufferCache[heatmap.Size] = hmb
			}
			heatmap.Heatmap = hmb
			err = ipts.Protocol.Read(hmb)
			break
		default:
			// ignored
			err = ipts.Protocol.Skip(uint32(report.Size))
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

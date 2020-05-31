package main

import (
	"unsafe"

	. "github.com/linux-surface/iptsd/processing/heatmap"
	. "github.com/linux-surface/iptsd/protocol"
)

var (
	heatmapCache map[uint16][]byte = make(map[uint16][]byte)
)

func IptsTouchHandleHeatmap(ipts *IptsContext, heatmap Heatmap) error {
	touch := ipts.Devices.Touch
	points := touch.Processor.Inputs(heatmap)

	for i := 0; i < len(points); i++ {
		p := points[i]

		touch.Device.Emit(EV_ABS, ABS_MT_SLOT, int32(i))
		touch.Device.Emit(EV_ABS, ABS_MT_TRACKING_ID, int32(p.Index))
		touch.Device.Emit(EV_ABS, ABS_MT_POSITION_X, int32(p.X))
		touch.Device.Emit(EV_ABS, ABS_MT_POSITION_Y, int32(p.Y))
	}

	err := touch.Device.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}

func IptsTouchHandleInput(ipts *IptsContext, frame IptsPayloadFrame) error {
	size := uint32(0)
	hm := Heatmap{}

	for size < frame.Size {
		report, err := ipts.Protocol.ReadReport()
		if err != nil {
			return err
		}

		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			height, err := ipts.Protocol.ReadByte()
			if err != nil {
				break
			}

			width, err := ipts.Protocol.ReadByte()
			if err != nil {
				break
			}

			hm.Height = int(height)
			hm.Width = int(width)

			err = ipts.Protocol.Skip(6)
			break
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			hmb, ok := heatmapCache[report.Size]
			if !ok {
				hmb = make([]byte, report.Size)
				heatmapCache[report.Size] = hmb
			}
			hm.Data = hmb
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

	if len(hm.Data) == 0 {
		return nil
	}

	if len(hm.Data) != hm.Width*hm.Height {
		return nil
	}

	err := IptsTouchHandleHeatmap(ipts, hm)
	if err != nil {
		return err
	}

	return nil
}

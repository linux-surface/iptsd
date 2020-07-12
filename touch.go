package main

import (
	"unsafe"

	. "github.com/linux-surface/iptsd/processing"
	. "github.com/linux-surface/iptsd/protocol"
)

func IptsTouchHandleHeatmap(ipts *IptsContext, heatmap *Heatmap) error {
	touch := ipts.Devices.Touch
	points := touch.Processor.Inputs(heatmap)
	blocked := false

	if ipts.Config.BlockOnPalm {
		for i := 0; i < len(points); i++ {
			blocked = blocked || points[i].IsPalm
		}
	}

	for i := 0; i < len(points); i++ {
		p := points[i]

		touch.Device.Emit(EV_ABS, ABS_MT_SLOT, int32(p.Slot))

		if p.Index != -1 && !p.IsStable {
			continue
		}

		if p.IsPalm || blocked {
			touch.Device.Emit(EV_ABS, ABS_MT_TRACKING_ID, -1)
			touch.Device.Emit(EV_ABS, ABS_MT_POSITION_X, 0)
			touch.Device.Emit(EV_ABS, ABS_MT_POSITION_Y, 0)

			continue
		}

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
	var hm *Heatmap

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

			dev := ipts.Devices.Touch
			hm = dev.Processor.GetHeatmap(int(width), int(height))

			err = ipts.Protocol.Skip(6)
			break
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			err = ipts.Protocol.Read(hm.Data)
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

	if hm == nil {
		return nil
	}

	err := IptsTouchHandleHeatmap(ipts, hm)
	if err != nil {
		return err
	}

	return nil
}

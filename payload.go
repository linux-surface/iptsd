package main

import (
	"bytes"
)

const (
	IPTS_PAYLOAD_FRAME_TYPE_STYLUS = 6
	IPTS_PAYLOAD_FRAME_TYPE_TOUCH  = 8

	IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM  = 0x0403
	IPTS_REPORT_TYPE_TOUCH_HEATMAP      = 0x0425
	IPTS_REPORT_TYPE_STYLUS_NO_TILT     = 0x0410
	IPTS_REPORT_TYPE_STYLUS_TILT        = 0x0461
	IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL = 0x0460
)

type IptsPayload struct {
	Counter  uint32
	Frames   uint32
	Reserved [4]uint8
}

type IptsPayloadFrame struct {
	Index    uint16
	Type     uint16
	Size     uint32
	Reserved [8]uint8
}

type IptsReport struct {
	Type uint16
	Size uint16
}

func IptsPayloadHandleInput(ipts *IPTS, buffer *bytes.Reader) {
	payload := IptsPayload{}

	IptsUtilsRead(buffer, &payload)

	for i := uint32(0); i < payload.Frames; i++ {
		frame := IptsPayloadFrame{}

		IptsUtilsRead(buffer, &frame)

		switch frame.Type {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			IptsStylusHandleInput(ipts, buffer, frame)
			break
		case IPTS_PAYLOAD_FRAME_TYPE_TOUCH:
			IptsTouchHandleInput(ipts, buffer, frame)
			break
		default:
			// ignored
			IptsUtilsSkip(buffer, frame.Size)
			break
		}
	}
}

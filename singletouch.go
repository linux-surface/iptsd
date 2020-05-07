package main

import (
	"bytes"
)

type IptsSingletouchReport struct {
	Touch uint8
	X     uint16
	Y     uint16
}

func IptsSingletouchHandleInput(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsSingletouchReport{}

	IptsUtilsRead(buffer, &report)

	ipts.Singletouch.Emit(EV_KEY, BTN_TOUCH, int32(report.Touch))
	ipts.Singletouch.Emit(EV_ABS, ABS_X, int32(report.X))
	ipts.Singletouch.Emit(EV_ABS, ABS_Y, int32(report.Y))

	ipts.Singletouch.Emit(EV_SYN, SYN_REPORT, 0)
}

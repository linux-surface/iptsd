package main

import (
	"bytes"
)

type IptsSingletouchReport struct {
	Touch uint8
	X     uint16
	Y     uint16
}

func IptsSingletouchHandleInput(ipts *IptsContext, buffer *bytes.Reader) error {
	singletouch := ipts.Devices.Singletouch
	report := IptsSingletouchReport{}

	err := IptsUtilsRead(buffer, &report)
	if err != nil {
		return err
	}

	singletouch.Emit(EV_KEY, BTN_TOUCH, int32(report.Touch))
	singletouch.Emit(EV_ABS, ABS_X, int32(report.X))
	singletouch.Emit(EV_ABS, ABS_Y, int32(report.Y))

	err = singletouch.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}

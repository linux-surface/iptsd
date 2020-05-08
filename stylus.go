package main

import (
	"bytes"
	"math"
	"unsafe"
)

type IptsStylusReportTilt struct {
	Elements uint8
	Reserved [3]uint8
}

type IptsStylusReportSerial struct {
	Elements uint8
	Reserved [3]uint8
	Serial   uint32
}

type IptsStylusReportData struct {
	Timestamp uint16
	Mode      uint16
	X         uint16
	Y         uint16
	Pressure  uint16
	Altitude  uint16
	Azimuth   uint16
	Reserved  uint16
}

type IptsStylusReportDataNoTilt struct {
	Reserved  [4]uint8
	Mode      uint8
	X         uint16
	Y         uint16
	Pressure  uint16
	Reserved2 uint8
}

const (
	IPTS_STYLUS_REPORT_MODE_PROX   = 1 << 0
	IPTS_STYLUS_REPORT_MODE_TOUCH  = 1 << 1
	IPTS_STYLUS_REPORT_MODE_BUTTON = 1 << 2
	IPTS_STYLUS_REPORT_MODE_RUBBER = 1 << 3
)

func IptsStylusHandleData(ipts *IPTS, data IptsStylusReportData) {
	stylus := ipts.ActiveStylus.Device

	prox := (data.Mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0
	touch := (data.Mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1
	button := (data.Mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2
	rubber := (data.Mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3

	btn_pen := prox * (1 - rubber)
	btn_rubber := prox * rubber

	tx := float64(0)
	ty := float64(0)

	if data.Altitude > 0 {
		alt := float64(data.Altitude) / 18000 * math.Pi
		azm := float64(data.Azimuth) / 18000 * math.Pi

		sin_alt := math.Sin(alt)
		sin_azm := math.Sin(azm)

		cos_alt := math.Cos(alt)
		cos_azm := math.Cos(azm)

		atan_x := math.Atan2(cos_alt, sin_alt*cos_azm)
		atan_y := math.Atan2(cos_alt, sin_alt*sin_azm)

		tx = 9000 - (atan_x * 4500 / (math.Pi / 4))
		ty = (atan_y * 4500 / (math.Pi / 4)) - 9000
	}

	stylus.Emit(EV_KEY, BTN_TOUCH, int32(touch))
	stylus.Emit(EV_KEY, BTN_TOOL_PEN, int32(btn_pen))
	stylus.Emit(EV_KEY, BTN_TOOL_RUBBER, int32(btn_rubber))
	stylus.Emit(EV_KEY, BTN_STYLUS, int32(button))

	stylus.Emit(EV_ABS, ABS_X, int32(data.X))
	stylus.Emit(EV_ABS, ABS_Y, int32(data.Y))
	stylus.Emit(EV_ABS, ABS_PRESSURE, int32(data.Pressure))
	stylus.Emit(EV_ABS, ABS_MISC, int32(data.Timestamp))

	stylus.Emit(EV_ABS, ABS_TILT_X, int32(tx))
	stylus.Emit(EV_ABS, ABS_TILT_Y, int32(ty))

	stylus.Emit(EV_SYN, SYN_REPORT, 0)
}

func IptsStylusHandleSerialChange(ipts *IPTS, serial uint32) {
	for _, stylus := range ipts.Styli {
		if stylus.Serial != serial {
			continue
		}

		ipts.ActiveStylus = stylus
		return
	}

	/*
	 * Before touching the screen for the first time, the stylus
	 * will report its serial as 0. Once you touch the screen,
	 * the serial will be reported correctly until you restart
	 * the machine.
	 */
	if ipts.ActiveStylus.Serial == 0 {
		ipts.ActiveStylus.Serial = serial
		return
	}

	newStylus := &IptsStylusDevice{
		Device: IptsDeviceCreateStylus(ipts),
		Serial: serial,
	}
	ipts.Styli = append(ipts.Styli, newStylus)
	ipts.ActiveStylus = newStylus
}

func IptsStylusHandleReportSerial(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportSerial{}

	IptsUtilsRead(buffer, &report)

	if ipts.ActiveStylus.Serial != report.Serial {
		IptsStylusHandleSerialChange(ipts, report.Serial)
	}

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportData{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, data)
	}
}

func IptsStylusHandleReportTilt(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportTilt{}

	IptsUtilsRead(buffer, &report)

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportData{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, data)
	}
}

func IptsStylusHandleReportNoTilt(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportSerial{}

	IptsUtilsRead(buffer, &report)

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportDataNoTilt{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, IptsStylusReportData{
			Mode:      uint16(data.Mode),
			X:         data.X,
			Y:         data.Y,
			Pressure:  data.Pressure * 4,
			Altitude:  0,
			Azimuth:   0,
			Timestamp: 0,
		})
	}
}

func IptsStylusHandleInput(ipts *IPTS, buffer *bytes.Reader, frame IptsPayloadFrame) {
	size := uint32(0)

	for size < frame.Size {
		report := IptsReport{}

		IptsUtilsRead(buffer, &report)
		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_STYLUS_NO_TILT:
			IptsStylusHandleReportNoTilt(ipts, buffer)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT:
			IptsStylusHandleReportTilt(ipts, buffer)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL:
			IptsStylusHandleReportSerial(ipts, buffer)
			break
		default:
			// ignored
			IptsUtilsSkip(buffer, uint32(report.Size))
			break
		}
	}
}

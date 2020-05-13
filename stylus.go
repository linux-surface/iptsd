package main

import (
	"math"
	"unsafe"

	. "github.com/linux-surface/iptsd/protocol"
)

var (
	xCache []uint16
	yCache []uint16
)

func IptsStylusHandleData(ipts *IptsContext, data IptsStylusData) error {
	stylus := ipts.Devices.ActiveStylus.Device

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

	if len(xCache) == 5 {
		xCache[0] = xCache[1]
		xCache[1] = xCache[2]
		xCache[2] = xCache[3]
		xCache[3] = xCache[4]
		xCache[4] = data.X

		yCache[0] = yCache[1]
		yCache[1] = yCache[2]
		yCache[2] = yCache[3]
		yCache[3] = yCache[4]
		yCache[4] = data.Y
	} else {
		xCache = append(xCache, data.X)
		yCache = append(yCache, data.Y)
	}

	x := uint16(0)
	y := uint16(0)
	for i := 0; i < len(xCache); i++ {
		x = x + xCache[i]
		y = y + yCache[i]
	}
	x = x / uint16(len(xCache))
	y = y / uint16(len(yCache))

	stylus.Emit(EV_KEY, BTN_TOUCH, int32(touch))
	stylus.Emit(EV_KEY, BTN_TOOL_PEN, int32(btn_pen))
	stylus.Emit(EV_KEY, BTN_TOOL_RUBBER, int32(btn_rubber))
	stylus.Emit(EV_KEY, BTN_STYLUS, int32(button))

	stylus.Emit(EV_ABS, ABS_X, int32(x))
	stylus.Emit(EV_ABS, ABS_Y, int32(y))
	stylus.Emit(EV_ABS, ABS_PRESSURE, int32(data.Pressure))
	stylus.Emit(EV_ABS, ABS_MISC, int32(data.Timestamp))

	stylus.Emit(EV_ABS, ABS_TILT_X, int32(tx))
	stylus.Emit(EV_ABS, ABS_TILT_Y, int32(ty))

	err := stylus.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}

func IptsStylusHandleSerialChange(ipts *IptsContext, serial uint32) error {
	for _, stylus := range ipts.Devices.Styli {
		if stylus.Serial != serial {
			continue
		}

		ipts.Devices.ActiveStylus = stylus
		return nil
	}

	/*
	 * Before touching the screen for the first time, the stylus
	 * will report its serial as 0. Once you touch the screen,
	 * the serial will be reported correctly until you restart
	 * the machine.
	 */
	if ipts.Devices.ActiveStylus.Serial == 0 {
		ipts.Devices.ActiveStylus.Serial = serial
		return nil
	}

	err := ipts.Devices.AddStylus(ipts.DeviceInfo, serial)
	if err != nil {
		return err
	}

	return nil
}

func IptsStylusHandleReportSerial(ipts *IptsContext) error {
	report, err := ipts.Protocol.ReadStylusReportSerial()
	if err != nil {
		return err
	}

	if ipts.Devices.ActiveStylus.Serial != report.Serial {
		err = IptsStylusHandleSerialChange(ipts, report.Serial)
		if err != nil {
			return err
		}
	}

	for i := uint8(0); i < report.Elements; i++ {
		data, err := ipts.Protocol.ReadStylusData()
		if err != nil {
			return err
		}

		err = IptsStylusHandleData(ipts, data)
		if err != nil {
			return err
		}
	}

	return nil
}

func IptsStylusHandleReportTilt(ipts *IptsContext) error {
	report, err := ipts.Protocol.ReadStylusReport()
	if err != nil {
		return err
	}

	for i := uint8(0); i < report.Elements; i++ {
		data, err := ipts.Protocol.ReadStylusData()
		if err != nil {
			return err
		}

		err = IptsStylusHandleData(ipts, data)
		if err != nil {
			return err
		}
	}

	return nil
}

func IptsStylusHandleReportNoTilt(ipts *IptsContext) error {
	report, err := ipts.Protocol.ReadStylusReportSerial()
	if err != nil {
		return err
	}

	if ipts.Devices.ActiveStylus.Serial != report.Serial {
		err = IptsStylusHandleSerialChange(ipts, report.Serial)
		if err != nil {
			return err
		}
	}

	for i := uint8(0); i < report.Elements; i++ {
		data, err := ipts.Protocol.ReadStylusDataNoTilt()
		if err != nil {
			return err
		}

		fullData := IptsStylusData{
			Mode:      uint16(data.Mode),
			X:         data.X,
			Y:         data.Y,
			Pressure:  data.Pressure * 4,
			Altitude:  0,
			Azimuth:   0,
			Timestamp: 0,
		}

		err = IptsStylusHandleData(ipts, fullData)
		if err != nil {
			return err
		}
	}

	return nil
}

func IptsStylusHandleInput(ipts *IptsContext, frame IptsPayloadFrame) error {
	size := uint32(0)

	for size < frame.Size {
		report, err := ipts.Protocol.ReadReport()
		if err != nil {
			return err
		}

		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_STYLUS_NO_TILT:
			err = IptsStylusHandleReportNoTilt(ipts)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT:
			err = IptsStylusHandleReportTilt(ipts)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL:
			err = IptsStylusHandleReportSerial(ipts)
			break
		default:
			// ignored
			err = ipts.Protocol.Skip(uint32(report.Size))
			break
		}

		if err != nil {
			return err
		}
	}

	return nil
}

package main

import (
	"unsafe"

	. "github.com/linux-surface/iptsd/protocol"
)

func IptsStylusHandleData(ipts *IptsContext, data IptsStylusData) error {
	stylus := ipts.Devices.ActiveStylus

	prox := (data.Mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0
	touch := (data.Mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1
	button := (data.Mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2
	rubber := (data.Mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3

	btn_pen := prox * (1 - rubber)
	btn_rubber := prox * rubber

	sx, sy := stylus.Processor.Smooth(int(data.X), int(data.Y))
	tx, ty := stylus.Processor.Tilt(int(data.Altitude), int(data.Azimuth))

	if prox == 0 {
		stylus.Processor.Flush()
	}

	stylus.Device.Emit(EV_KEY, BTN_TOUCH, int32(touch))
	stylus.Device.Emit(EV_KEY, BTN_TOOL_PEN, int32(btn_pen))
	stylus.Device.Emit(EV_KEY, BTN_TOOL_RUBBER, int32(btn_rubber))
	stylus.Device.Emit(EV_KEY, BTN_STYLUS, int32(button))

	stylus.Device.Emit(EV_ABS, ABS_X, int32(sx))
	stylus.Device.Emit(EV_ABS, ABS_Y, int32(sy))
	stylus.Device.Emit(EV_ABS, ABS_PRESSURE, int32(data.Pressure))
	stylus.Device.Emit(EV_ABS, ABS_MISC, int32(data.Timestamp))

	stylus.Device.Emit(EV_ABS, ABS_TILT_X, int32(tx))
	stylus.Device.Emit(EV_ABS, ABS_TILT_Y, int32(ty))

	err := stylus.Device.Emit(EV_SYN, SYN_REPORT, 0)
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

	err := ipts.Devices.AddStylus(ipts, serial)
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

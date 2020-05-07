package main

func IptsDeviceCreateStylus(ipts *IPTS) *UinputDevice {
	dev := &UinputDevice{
		Name:    "IPTS Stylus",
		Vendor:  ipts.DeviceInfo.Vendor,
		Product: ipts.DeviceInfo.Device,
		Version: uint16(ipts.DeviceInfo.FwRevision),
	}

	dev.Open()

	dev.SetEvbit(EV_KEY)
	dev.SetEvbit(EV_ABS)

	dev.SetPropbit(INPUT_PROP_DIRECT)
	dev.SetPropbit(INPUT_PROP_POINTER)

	dev.SetKeybit(BTN_TOUCH)
	dev.SetKeybit(BTN_STYLUS)
	dev.SetKeybit(BTN_TOOL_PEN)
	dev.SetKeybit(BTN_TOOL_RUBBER)

	dev.SetAbsInfo(ABS_X, UinputAbsInfo{
		Minimum:    0,
		Maximum:    9600,
		Resolution: 34,
	})

	dev.SetAbsInfo(ABS_Y, UinputAbsInfo{
		Minimum:    0,
		Maximum:    7200,
		Resolution: 38,
	})

	dev.SetAbsInfo(ABS_PRESSURE, UinputAbsInfo{
		Minimum: 0,
		Maximum: 4096,
	})

	dev.SetAbsInfo(ABS_TILT_X, UinputAbsInfo{
		Minimum:    -9000,
		Maximum:    9000,
		Resolution: 5730,
	})

	dev.SetAbsInfo(ABS_TILT_Y, UinputAbsInfo{
		Minimum:    -9000,
		Maximum:    9000,
		Resolution: 5730,
	})

	dev.SetAbsInfo(ABS_MISC, UinputAbsInfo{
		Minimum: 0,
		Maximum: 65535,
	})

	dev.Create()

	return dev
}

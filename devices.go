package main

type IptsStylusDevice struct {
	Serial uint32
	Device *UinputDevice
}

type IptsDevices struct {
	Singletouch *UinputDevice
	Touch       *UinputDevice

	ActiveStylus *IptsStylusDevice
	Styli        []*IptsStylusDevice
}

func IptsDevicesCreateStylus(info *IptsDeviceInfo) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Stylus",
		Vendor:  info.Vendor,
		Product: info.Device,
		Version: uint16(info.FwRevision),
	}

	err := dev.Open()
	if err != nil {
		return nil, err
	}

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

	err = dev.Create()
	if err != nil {
		return nil, err
	}

	return dev, nil
}

func IptsDevicesCreateTouch(info *IptsDeviceInfo) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Touch",
		Vendor:  info.Vendor,
		Product: info.Device,
		Version: uint16(info.FwRevision),
	}

	err := dev.Open()
	if err != nil {
		return nil, err
	}

	dev.SetEvbit(EV_ABS)
	dev.SetPropbit(INPUT_PROP_DIRECT)

	dev.SetAbsInfo(ABS_MT_SLOT, UinputAbsInfo{
		Minimum: 0,
		Maximum: 10,
	})

	dev.SetAbsInfo(ABS_MT_TRACKING_ID, UinputAbsInfo{
		Minimum: 0,
		Maximum: 10,
	})

	dev.SetAbsInfo(ABS_MT_POSITION_X, UinputAbsInfo{
		Minimum: 0,
		Maximum: 32767,
	})

	dev.SetAbsInfo(ABS_MT_POSITION_Y, UinputAbsInfo{
		Minimum: 0,
		Maximum: 32767,
	})

	err = dev.Create()
	if err != nil {
		return nil, err
	}

	return dev, nil
}

func IptsDevicesCreateSingletouch(info *IptsDeviceInfo) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Singletouch",
		Vendor:  info.Vendor,
		Product: info.Device,
		Version: uint16(info.FwRevision),
	}

	err := dev.Open()
	if err != nil {
		return nil, err
	}

	dev.SetEvbit(EV_ABS)
	dev.SetEvbit(EV_KEY)

	dev.SetPropbit(INPUT_PROP_DIRECT)
	dev.SetKeybit(BTN_TOUCH)

	dev.SetAbsInfo(ABS_X, UinputAbsInfo{
		Minimum:    0,
		Maximum:    32767,
		Resolution: 112,
	})

	dev.SetAbsInfo(ABS_Y, UinputAbsInfo{
		Minimum:    0,
		Maximum:    32767,
		Resolution: 199,
	})

	err = dev.Create()
	if err != nil {
		return nil, err
	}

	return dev, nil
}

func (devices *IptsDevices) AddStylus(info *IptsDeviceInfo, serial uint32) error {
	stylus, err := IptsDevicesCreateStylus(info)
	if err != nil {
		return err
	}

	devices.ActiveStylus = &IptsStylusDevice{
		Device: stylus,
		Serial: serial,
	}
	devices.Styli = append(devices.Styli, devices.ActiveStylus)

	return nil
}

func (devices *IptsDevices) Create(info *IptsDeviceInfo) error {
	singletouch, err := IptsDevicesCreateSingletouch(info)
	if err != nil {
		return err
	}

	touch, err := IptsDevicesCreateTouch(info)
	if err != nil {
		return err
	}

	devices.Singletouch = singletouch
	devices.Touch = touch
	devices.AddStylus(info, uint32(0))

	return nil
}

func (devices *IptsDevices) Destroy() error {
	err := devices.Singletouch.Destroy()
	if err != nil {
		return err
	}

	for _, stylus := range devices.Styli {
		err = stylus.Device.Destroy()
		if err != nil {
			return err
		}
	}

	return nil
}

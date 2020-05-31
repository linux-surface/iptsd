package main

import (
	. "github.com/linux-surface/iptsd/processing"
)

type IptsStylusDevice struct {
	Serial    uint32
	Device    *UinputDevice
	Processor *StylusProcessor
}

type IptsTouchDevice struct {
	Device    *UinputDevice
	Processor *TouchProcessor
}

type IptsDevices struct {
	Touch *IptsTouchDevice

	ActiveStylus *IptsStylusDevice
	Styli        []*IptsStylusDevice
}

func IptsDevicesCreateStylus(info *IptsDeviceInfo) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Stylus",
		Vendor:  info.Vendor,
		Product: info.Product,
		Version: uint16(info.Version),
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
		Product: info.Product,
		Version: uint16(info.Version),
	}

	err := dev.Open()
	if err != nil {
		return nil, err
	}

	dev.SetEvbit(EV_ABS)
	dev.SetPropbit(INPUT_PROP_DIRECT)

	dev.SetAbsInfo(ABS_MT_SLOT, UinputAbsInfo{
		Minimum: 0,
		Maximum: int32(info.MaxTouchPoints),
	})

	dev.SetAbsInfo(ABS_MT_TRACKING_ID, UinputAbsInfo{
		Minimum: 0,
		Maximum: int32(info.MaxTouchPoints),
	})

	dev.SetAbsInfo(ABS_MT_POSITION_X, UinputAbsInfo{
		Minimum: 0,
		Maximum: 9600,
	})

	dev.SetAbsInfo(ABS_MT_POSITION_Y, UinputAbsInfo{
		Minimum: 0,
		Maximum: 7200,
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

	processor := &StylusProcessor{}
	processor.Flush()

	devices.ActiveStylus = &IptsStylusDevice{
		Device:    stylus,
		Serial:    serial,
		Processor: processor,
	}
	devices.Styli = append(devices.Styli, devices.ActiveStylus)

	return nil
}

func (devices *IptsDevices) Create(info *IptsDeviceInfo, quirks *IptsQuirks) error {
	touch, err := IptsDevicesCreateTouch(info)
	if err != nil {
		return err
	}

	processor := &TouchProcessor{
		InvertX:        quirks.Has(IPTS_QUIRKS_HEATMAP_INVERT_X),
		InvertY:        quirks.Has(IPTS_QUIRKS_HEATMAP_INVERT_Y),
		MaxTouchPoints: int(info.MaxTouchPoints),
	}

	devices.Touch = &IptsTouchDevice{
		Device:    touch,
		Processor: processor,
	}

	devices.AddStylus(info, uint32(0))

	return nil
}

func (devices *IptsDevices) Destroy() error {
	err := devices.Touch.Device.Destroy()
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

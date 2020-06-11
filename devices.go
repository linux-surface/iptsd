package main

import (
	"math"

	. "github.com/linux-surface/iptsd/processing"
	"github.com/pkg/errors"
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

func IptsDevicesGetRes(virt int, phys int) int32 {
	res := float64(virt*10) / float64(phys)
	return int32(math.Round(res))
}

func IptsDevicesCreateStylus(ipts *IptsContext) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Stylus",
		Vendor:  ipts.DeviceInfo.Vendor,
		Product: ipts.DeviceInfo.Product,
		Version: uint16(ipts.DeviceInfo.Version),
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
		Resolution: IptsDevicesGetRes(9600, ipts.Config.Width),
	})

	dev.SetAbsInfo(ABS_Y, UinputAbsInfo{
		Minimum:    0,
		Maximum:    7200,
		Resolution: IptsDevicesGetRes(7200, ipts.Config.Height),
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

func IptsDevicesCreateTouch(ipts *IptsContext) (*UinputDevice, error) {
	dev := &UinputDevice{
		Name:    "IPTS Touch",
		Vendor:  ipts.DeviceInfo.Vendor,
		Product: ipts.DeviceInfo.Product,
		Version: uint16(ipts.DeviceInfo.Version),
	}

	err := dev.Open()
	if err != nil {
		return nil, err
	}

	dev.SetEvbit(EV_ABS)
	dev.SetPropbit(INPUT_PROP_DIRECT)

	dev.SetAbsInfo(ABS_MT_SLOT, UinputAbsInfo{
		Minimum: 0,
		Maximum: int32(ipts.DeviceInfo.MaxTouchPoints),
	})

	dev.SetAbsInfo(ABS_MT_TRACKING_ID, UinputAbsInfo{
		Minimum: 0,
		Maximum: int32(ipts.DeviceInfo.MaxTouchPoints),
	})

	dev.SetAbsInfo(ABS_MT_POSITION_X, UinputAbsInfo{
		Minimum:    0,
		Maximum:    9600,
		Resolution: IptsDevicesGetRes(9600, ipts.Config.Width),
	})

	dev.SetAbsInfo(ABS_MT_POSITION_Y, UinputAbsInfo{
		Minimum:    0,
		Maximum:    7200,
		Resolution: IptsDevicesGetRes(7200, ipts.Config.Height),
	})

	err = dev.Create()
	if err != nil {
		return nil, err
	}

	return dev, nil
}

func (devices *IptsDevices) AddStylus(ipts *IptsContext, serial uint32) error {
	stylus, err := IptsDevicesCreateStylus(ipts)
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

func (devices *IptsDevices) Create(ipts *IptsContext) error {
	if ipts.Config.Width == 0 || ipts.Config.Height == 0 {
		return errors.Errorf("Display size is 0")
	}

	touch, err := IptsDevicesCreateTouch(ipts)
	if err != nil {
		return err
	}

	processor := &TouchProcessor{
		InvertX:        ipts.Config.InvertX,
		InvertY:        ipts.Config.InvertY,
		MaxTouchPoints: int(ipts.DeviceInfo.MaxTouchPoints),
	}

	devices.Touch = &IptsTouchDevice{
		Device:    touch,
		Processor: processor,
	}

	devices.AddStylus(ipts, uint32(0))

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

package main

// #define type __type
// #include <linux/uinput.h>
// #include <stdlib.h>
// #include <string.h>
import "C"
import (
	"encoding/binary"
	"os"
	"unsafe"

	"github.com/pkg/errors"
)

var (
	UINPUT_DEVICE = "/dev/uinput"

	UI_DEV_SETUP   = uintptr(C.UI_DEV_SETUP)
	UI_DEV_CREATE  = uintptr(C.UI_DEV_CREATE)
	UI_DEV_DESTROY = uintptr(C.UI_DEV_DESTROY)
	UI_SET_EVBIT   = uintptr(C.UI_SET_EVBIT)
	UI_SET_KEYBIT  = uintptr(C.UI_SET_KEYBIT)
	UI_SET_ABSBIT  = uintptr(C.UI_SET_ABSBIT)
	UI_SET_PROPBIT = uintptr(C.UI_SET_PROPBIT)
	UI_ABS_SETUP   = uintptr(C.UI_ABS_SETUP)

	BUS_VIRTUAL = uint32(C.BUS_VIRTUAL)

	EV_ABS = uint16(C.EV_ABS)
	EV_KEY = uint16(C.EV_KEY)
	EV_SYN = uint16(C.EV_SYN)

	SYN_REPORT = uint16(C.SYN_REPORT)

	INPUT_PROP_DIRECT  = uint16(C.INPUT_PROP_DIRECT)
	INPUT_PROP_POINTER = uint16(C.INPUT_PROP_POINTER)

	BTN_TOUCH       = uint16(C.BTN_TOUCH)
	BTN_STYLUS      = uint16(C.BTN_STYLUS)
	BTN_TOOL_PEN    = uint16(C.BTN_TOOL_PEN)
	BTN_TOOL_RUBBER = uint16(C.BTN_TOOL_RUBBER)

	ABS_X        = uint16(C.ABS_X)
	ABS_Y        = uint16(C.ABS_Y)
	ABS_TILT_X   = uint16(C.ABS_TILT_X)
	ABS_TILT_Y   = uint16(C.ABS_TILT_Y)
	ABS_PRESSURE = uint16(C.ABS_PRESSURE)
	ABS_MISC     = uint16(C.ABS_MISC)
)

type UinputDevice struct {
	file    *os.File
	created bool

	Name    string
	Vendor  uint16
	Product uint16
	Version uint16
}

type UinputAbsInfo struct {
	Value      int32
	Minimum    int32
	Maximum    int32
	Fuzz       int32
	Flat       int32
	Resolution int32
}

func (dev *UinputDevice) Open() error {
	file, err := os.OpenFile(UINPUT_DEVICE, os.O_WRONLY, 0660)
	if err != nil {
		return errors.WithStack(err)
	}

	dev.file = file
	return nil
}

func (dev *UinputDevice) Create() error {
	setup := C.struct_uinput_setup{}
	setup.id.bustype = C.ushort(BUS_VIRTUAL)
	setup.id.vendor = C.ushort(dev.Vendor)
	setup.id.product = C.ushort(dev.Product)
	setup.id.version = C.ushort(dev.Version)

	name := C.CString(dev.Name)
	C.strcpy(&setup.name[0], name)
	C.free(unsafe.Pointer(name))

	err := ioctl(dev.file, UI_DEV_SETUP, uintptr(unsafe.Pointer(&setup)))
	if err != nil {
		return err
	}

	err = ioctl(dev.file, UI_DEV_CREATE, uintptr(0))
	if err != nil {
		return err
	}

	return nil
}

func (dev *UinputDevice) Destroy() error {
	err := ioctl(dev.file, UI_DEV_DESTROY, uintptr(0))
	if err != nil {
		return err
	}

	err = dev.file.Close()
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (dev *UinputDevice) SetEvbit(value uint16) error {
	err := ioctl(dev.file, UI_SET_EVBIT, uintptr(value))
	if err != nil {
		return err
	}

	return nil
}

func (dev *UinputDevice) SetKeybit(value uint16) error {
	err := ioctl(dev.file, UI_SET_KEYBIT, uintptr(value))
	if err != nil {
		return err
	}

	return nil
}

func (dev *UinputDevice) SetPropbit(value uint16) error {
	err := ioctl(dev.file, UI_SET_PROPBIT, uintptr(value))
	if err != nil {
		return err
	}

	return nil
}

func (dev *UinputDevice) SetAbsInfo(axis uint16, info UinputAbsInfo) error {
	setup := C.struct_uinput_abs_setup{}
	setup.code = C.ushort(axis)
	setup.absinfo.value = C.int(info.Value)
	setup.absinfo.minimum = C.int(info.Minimum)
	setup.absinfo.maximum = C.int(info.Maximum)
	setup.absinfo.fuzz = C.int(info.Fuzz)
	setup.absinfo.flat = C.int(info.Flat)
	setup.absinfo.resolution = C.int(info.Resolution)

	err := ioctl(dev.file, UI_ABS_SETUP, uintptr(unsafe.Pointer(&setup)))
	if err != nil {
		return err
	}

	return nil
}

func (dev *UinputDevice) Emit(event uint16, code uint16, value int32) error {
	ie := C.struct_input_event{}
	ie.__type = C.ushort(event)
	ie.code = C.ushort(code)
	ie.value = C.int(value)

	err := binary.Write(dev.file, binary.LittleEndian, &ie)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

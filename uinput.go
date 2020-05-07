package main

// #include <linux/uinput.h>
// #include <stdlib.h>
// #include <string.h>
import "C"
import (
	"os"
	"unsafe"
)

var (
	UINPUT_DEVICE = "/dev/uinput"

	UI_DEV_SETUP   = uintptr(C.UI_DEV_SETUP)
	UI_DEV_CREATE  = uintptr(C.UI_DEV_CREATE)
	UI_DEV_DESTROY = uintptr(C.UI_DEV_DESTROY)
	UI_SET_EVBIT   = uintptr(C.UI_SET_EVBIT)
	UI_SET_KEYBIT  = uintptr(C.UI_SET_KEYBIT)
)

type UinputSetup C.struct_uinput_setup
type InputEvent C.struct_input_event

type UinputDevice struct {
	file    *os.File
	created bool

	Name    string
	Vendor  uint16
	Product uint16
	Version uint16
	Bustype uint16
}

func (dev *UinputDevice) Open() {
	file, err := os.OpenFile(UINPUT_DEVICE, os.O_WRONLY, 0660)
	if err != nil {
		panic(err)
	}

	dev.file = file
}

func (dev *UinputDevice) Create() {
	if dev.file == nil {
		return
	}

	if dev.created {
		return
	}

	setup := UinputSetup{}
	setup.id.bustype = C.ushort(dev.Bustype)
	setup.id.vendor = C.ushort(dev.Vendor)
	setup.id.product = C.ushort(dev.Product)
	setup.id.version = C.ushort(dev.Version)

	name := C.CString(dev.Name)
	C.strcpy(&setup.name[0], name)
	C.free(unsafe.Pointer(name))

	err := ioctl(dev.file, UI_DEV_SETUP, uintptr(unsafe.Pointer(&setup)))
	if err != nil {
		panic(err)
	}

	err = ioctl(dev.file, UI_DEV_CREATE, uintptr(0))
	if err != nil {
		panic(err)
	}

	dev.created = true
}

func (dev *UinputDevice) Destroy() {
	if dev.file == nil {
		return
	}

	if !dev.created {
		return
	}

	err := ioctl(dev.file, UI_DEV_DESTROY, uintptr(0))
	if err != nil {
		panic(err)
	}

	dev.created = false
}

func (dev *UinputDevice) Close() {
	if dev.file == nil {
		return
	}

	dev.Destroy()

	err := dev.file.Close()
	if err != nil {
		panic(err)
	}
}

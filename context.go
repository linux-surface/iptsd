package main

import (
	"os"
	"unsafe"
)

var (
	IPTS_DEVICE = "/dev/ipts"

	IPTS_UAPI_INFO  = _IOR(0x86, 0x01, unsafe.Sizeof(IptsDeviceInfo{}))
	IPTS_UAPI_START = _IO(0x86, 0x02)
	IPTS_UAPI_STOP  = _IO(0x86, 0x03)
)

type IPTS struct {
	file    *os.File
	started bool

	DeviceInfo IptsDeviceInfo
}

func (ipts *IPTS) Open() {
	file, err := os.OpenFile(IPTS_DEVICE, os.O_RDONLY, 0660)
	if err != nil {
		panic(err)
	}

	ipts.file = file
	ipts.started = false

	device := IptsDeviceInfo{}
	err = ioctl(file, IPTS_UAPI_INFO, uintptr(unsafe.Pointer(&device)))
	if err != nil {
		file.Close()
		panic(err)
	}

	ipts.DeviceInfo = device
}

func (ipts *IPTS) Close() {
	ipts.Stop()
	err := ipts.file.Close()
	if err != nil {
		panic(err)
	}
}

func (ipts *IPTS) Start() {
	if ipts.started {
		return
	}

	ipts.started = true
	err := ioctl(ipts.file, IPTS_UAPI_START, uintptr(0))
	if err != nil {
		panic(err)
	}
}

func (ipts *IPTS) Stop() {
	if !ipts.started {
		return
	}

	ipts.started = false
	err := ioctl(ipts.file, IPTS_UAPI_STOP, uintptr(0))
	if err != nil {
		panic(err)
	}
}

func (ipts *IPTS) Read(buffer []byte) int {
	n, _ := ipts.file.Read(buffer)
	return n
}

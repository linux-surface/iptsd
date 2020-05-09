package main

import (
	"os"
	"syscall"
	"unsafe"
)

var (
	IPTS_DEVICE = "/dev/ipts"

	IPTS_UAPI_INFO  = _IOR(0x86, 0x01, unsafe.Sizeof(IptsDeviceInfo{}))
	IPTS_UAPI_START = _IO(0x86, 0x02)
	IPTS_UAPI_STOP  = _IO(0x86, 0x03)
)

type IptsDeviceInfo struct {
	Vendor       uint16
	Device       uint16
	HwRevision   uint32
	FwRevision   uint32
	DataSize     uint32
	FeedbackSize uint32
	Reserved     [24]uint8
}

type IptsStylusDevice struct {
	Serial uint32
	Device *UinputDevice
}

type IPTS struct {
	started bool
	epoll   *Epoll
	file    *os.File
	events  []syscall.EpollEvent

	DeviceInfo  IptsDeviceInfo
	Singletouch *UinputDevice

	ActiveStylus *IptsStylusDevice
	Styli        []*IptsStylusDevice
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

	epoll := &Epoll{}
	epoll.Create()
	epoll.Listen(file, syscall.EPOLLIN|syscall.EPOLLHUP)

	ipts.epoll = epoll
	ipts.events = make([]syscall.EpollEvent, 10)
}

func (ipts *IPTS) Start() {
	if ipts.file == nil {
		return
	}

	if ipts.started {
		return
	}

	ipts.Singletouch = IptsDeviceCreateSingletouch(ipts)

	ipts.ActiveStylus = &IptsStylusDevice{
		Device: IptsDeviceCreateStylus(ipts),
	}
	ipts.Styli = []*IptsStylusDevice{ipts.ActiveStylus}

	err := ioctl(ipts.file, IPTS_UAPI_START, uintptr(0))
	if err != nil {
		panic(err)
	}

	ipts.started = true
}

func (ipts *IPTS) Stop() {
	if ipts.file == nil {
		return
	}

	if !ipts.started {
		return
	}

	ipts.Singletouch.Close()
	for _, stylus := range ipts.Styli {
		stylus.Device.Close()
	}

	ioctl(ipts.file, IPTS_UAPI_STOP, uintptr(0))
	ipts.started = false
}

func (ipts *IPTS) Close() {
	if ipts.file == nil {
		return
	}

	ipts.Stop()
	ipts.file.Close()
	ipts.epoll.Destroy()
}

func (ipts *IPTS) Restart() bool {
	if _, err := os.Stat(IPTS_DEVICE); os.IsNotExist(err) {
		return false
	}

	ipts.Close()
	ipts.Open()
	ipts.Start()
	return true
}

func (ipts *IPTS) Read(buffer []byte) int {
	if ipts.file == nil {
		return 0
	}

	if !ipts.started {
		return 0
	}

	ipts.epoll.Wait(ipts.events)

	for _, event := range ipts.events {
		hup := event.Events&syscall.EPOLLHUP > 0
		in := event.Events&syscall.EPOLLIN > 0

		if hup {
			ipts.Restart()
			return 0
		}

		if in {
			n, _ := ipts.file.Read(buffer)
			return n
		}
	}

	return 0
}

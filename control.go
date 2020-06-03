package main

import (
	"io"
	"os"
	"syscall"
	"time"
	"unsafe"

	"github.com/pkg/errors"
)

var (
	IPTS_DEVICE = "/dev/ipts"

	IPTS_UAPI_INFO  = _IOR(0x86, 0x01, unsafe.Sizeof(IptsDeviceInfo{}))
	IPTS_UAPI_START = _IO(0x86, 0x02)
	IPTS_UAPI_STOP  = _IO(0x86, 0x03)
)

type IptsDeviceInfo struct {
	Vendor         uint16
	Product        uint16
	Version        uint32
	BufferSize     uint32
	MaxTouchPoints uint8
	Reserved       [19]uint8
}

type IptsControl struct {
	epoll  *Epoll
	file   *os.File
	events []syscall.EpollEvent
}

func (ipts *IptsControl) Start() error {
	file, err := os.OpenFile(IPTS_DEVICE, os.O_RDONLY, 0660)
	if err != nil {
		return errors.WithStack(err)
	}

	ipts.file = file
	ipts.epoll = &Epoll{}

	err = ipts.epoll.Create()
	if err != nil {
		return err
	}

	err = ipts.epoll.Listen(file, syscall.EPOLLIN)
	if err != nil {
		return err
	}

	ipts.events = make([]syscall.EpollEvent, 10)

	err = ioctl(ipts.file, IPTS_UAPI_START, uintptr(0))
	if err != nil {
		return err
	}

	return nil
}

func (ipts *IptsControl) Stop() error {
	err := ioctl(ipts.file, IPTS_UAPI_STOP, uintptr(0))
	if err != nil {
		return err
	}

	err = ipts.file.Close()
	if err != nil {
		return errors.WithStack(err)
	}

	err = ipts.epoll.Destroy()
	if err != nil {
		return err
	}

	return nil
}

func (ipts *IptsControl) Restart() error {
	ipts.Stop()

	err := ipts.Start()
	if err != nil {
		return err
	}

	return nil
}

func (ipts *IptsControl) DeviceInfo() (*IptsDeviceInfo, error) {
	info := &IptsDeviceInfo{}

	ptr := unsafe.Pointer(info)
	err := ioctl(ipts.file, IPTS_UAPI_INFO, uintptr(ptr))
	if err != nil {
		return nil, err
	}

	return info, nil
}

func (ipts *IptsControl) Read(buffer []byte) (int, error) {
	ipts.epoll.Wait(ipts.events)

	for _, event := range ipts.events {
		hup := (event.Events & syscall.EPOLLHUP) > 0
		in := (event.Events & syscall.EPOLLIN) > 0

		if hup {
			var err error

			for i := 0; i < 5; i++ {
				err = ipts.Restart()
				if err == nil {
					return 0, nil
				}

				time.Sleep(200 * time.Millisecond)
			}

			return 0, err
		}

		if in {
			n, err := ipts.file.Read(buffer)
			if err != nil && err != io.EOF {
				return 0, errors.WithStack(err)
			}

			return n, nil
		}
	}

	return 0, nil
}

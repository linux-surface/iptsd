package main

// #include "ipts.h"
import "C"
import (
	"fmt"
	"os"
	"unsafe"

	"github.com/pkg/errors"
)

var (
	IPTS_DEVICE = "/dev/ipts/%d"

	IPTS_BUFFERS = int(C.IPTS_BUFFERS)

	IPTS_IOCTL_GET_DEVICE_INFO = uintptr(C.IPTS_IOCTL_GET_DEVICE_INFO)
	IPTS_IOCTL_GET_DOORBELL    = uintptr(C.IPTS_IOCTL_GET_DOORBELL)
	IPTS_IOCTL_SEND_FEEDBACK   = uintptr(C.IPTS_IOCTL_SEND_FEEDBACK)
)

type IptsDeviceInfo struct {
	Vendor         uint16
	Product        uint16
	Version        uint32
	BufferSize     uint32
	MaxTouchPoints uint8
}

type IptsControl struct {
	files           []*os.File
	deviceInfo      C.struct_ipts_device_info
	finalDoorbell   uint32
	currentDoorbell uint32
}

func (ipts *IptsControl) Start() error {
	ipts.files = make([]*os.File, IPTS_BUFFERS)
	ipts.currentDoorbell = 0
	ipts.deviceInfo = C.struct_ipts_device_info{}

	for i := 0; i < IPTS_BUFFERS; i++ {
		name := fmt.Sprintf(IPTS_DEVICE, i)

		file, err := os.OpenFile(name, os.O_RDONLY, 0660)
		if err != nil {
			return errors.WithStack(err)
		}

		ipts.files[i] = file

		// Send feedback for every buffer to flush leftover data.
		ipts.SendFeedbackFile(file)
	}

	doorbell, err := ipts.Doorbell()
	if err != nil {
		return err
	}

	ipts.currentDoorbell = doorbell

	return nil
}

func (ipts *IptsControl) CurrentFile() *os.File {
	index := ipts.currentDoorbell % uint32(IPTS_BUFFERS)
	return ipts.files[index]
}

func (ipts *IptsControl) DeviceInfo() (IptsDeviceInfo, error) {
	ptr := uintptr(unsafe.Pointer(&ipts.deviceInfo))
	err := ioctl(ipts.CurrentFile(), IPTS_IOCTL_GET_DEVICE_INFO, ptr)
	if err != nil {
		return IptsDeviceInfo{}, err
	}

	return IptsDeviceInfo{
		Vendor:         uint16(ipts.deviceInfo.vendor),
		Product:        uint16(ipts.deviceInfo.product),
		Version:        uint32(ipts.deviceInfo.version),
		BufferSize:     uint32(ipts.deviceInfo.buffer_size),
		MaxTouchPoints: uint8(ipts.deviceInfo.max_contacts),
	}, nil
}

func (ipts *IptsControl) Doorbell() (uint32, error) {
	ptr := uintptr(unsafe.Pointer(&ipts.finalDoorbell))
	err := ioctl(ipts.CurrentFile(), IPTS_IOCTL_GET_DOORBELL, ptr)
	if err != nil {
		return 0, err
	}

	return ipts.finalDoorbell, nil
}

func (ipts *IptsControl) CurrentDoorbell() uint32 {
	return ipts.currentDoorbell
}

func (ipts *IptsControl) IncrementDoorbell() {
	ipts.currentDoorbell++
}

func (ipts *IptsControl) SendFeedbackFile(file *os.File) error {
	err := ioctl(file, IPTS_IOCTL_SEND_FEEDBACK, uintptr(0))
	if err != nil {
		return err
	}

	return nil
}

func (ipts *IptsControl) SendFeedback() error {
	return ipts.SendFeedbackFile(ipts.CurrentFile())
}

func (ipts *IptsControl) Read(buffer []byte) (int, error) {
	n, err := ipts.CurrentFile().Read(buffer)
	if err != nil {
		return 0, errors.WithStack(err)
	}

	return n, nil
}

func (ipts *IptsControl) Stop() error {
	for i := 0; i < IPTS_BUFFERS; i++ {
		err := ipts.files[i].Close()
		if err != nil {
			return errors.WithStack(err)
		}
	}

	return nil
}

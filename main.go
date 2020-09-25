package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	. "github.com/linux-surface/iptsd/protocol"
)

type IptsContext struct {
	Control    *IptsControl
	Devices    *IptsDevices
	Protocol   *IptsProtocol
	Config     *IptsConfig
	DeviceInfo IptsDeviceInfo
}

func Shutdown(ipts *IptsContext) {
	ipts.Devices.Destroy()
	ipts.Control.Stop()
	os.Exit(1)
}

func HandleError(ipts *IptsContext, err error) {
	fmt.Printf("%+v\n", err)
	Shutdown(ipts)
}

func main() {
	ipts := &IptsContext{}
	ipts.Control = &IptsControl{}
	ipts.Protocol = &IptsProtocol{}
	ipts.Devices = &IptsDevices{}
	ipts.Config = &IptsConfig{}

	sc := make(chan os.Signal, 1)
	signal.Notify(sc, syscall.SIGINT, syscall.SIGTERM, os.Interrupt)
	go func() {
		<-sc
		Shutdown(ipts)
	}()

	err := ipts.Control.Start()
	if err != nil {
		HandleError(ipts, err)
	}

	info, err := ipts.Control.DeviceInfo()
	if err != nil {
		HandleError(ipts, err)
	}

	ipts.DeviceInfo = info

	fmt.Printf("Connected to device %04x:%04x\n",
		ipts.DeviceInfo.Vendor, ipts.DeviceInfo.Product)

	err = ipts.Config.Load(info)
	if err != nil {
		HandleError(ipts, err)
	}

	err = ipts.Devices.Create(ipts)
	if err != nil {
		HandleError(ipts, err)
	}

	buffer := make([]byte, ipts.DeviceInfo.BufferSize)
	ipts.Protocol.Create(buffer)

	timeout := time.Now().Add(5 * time.Second)

	for {
		doorbell, err := ipts.Control.Doorbell()
		if err != nil {
			HandleError(ipts, err)
		}

		for doorbell != ipts.Control.CurrentDoorbell() {
			timeout = time.Now().Add(5 * time.Second)

			count, err := ipts.Control.Read(buffer)
			if err != nil {
				HandleError(ipts, err)
			}

			if count == 0 {
				continue
			}

			err = IptsDataHandleInput(ipts)
			if err != nil {
				HandleError(ipts, err)
			}

			err = ipts.Protocol.Reset()
			if err != nil {
				HandleError(ipts, err)
			}

			err = ipts.Control.SendFeedback()
			if err != nil {
				HandleError(ipts, err)
			}

			ipts.Control.IncrementDoorbell()
		}

		if time.Now().Before(timeout) {
			time.Sleep(10 * time.Millisecond)
		} else {
			time.Sleep(200 * time.Millisecond)
		}
	}
}

package main

import (
	"fmt"
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

func main() {
	ipts := &IptsContext{}
	ipts.Control = &IptsControl{}
	ipts.Protocol = &IptsProtocol{}
	ipts.Devices = &IptsDevices{}
	ipts.Config = &IptsConfig{}

	err := ipts.Control.Start()
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	info, err := ipts.Control.DeviceInfo()
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	ipts.DeviceInfo = info

	fmt.Printf("Connected to device %04x:%04x\n",
		ipts.DeviceInfo.Vendor, ipts.DeviceInfo.Product)

	err = ipts.Config.Load(info)
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	err = ipts.Devices.Create(ipts)
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	buffer := make([]byte, ipts.DeviceInfo.BufferSize)
	ipts.Protocol.Create(buffer)

	timeout := time.Now().Add(5 * time.Second)

	for {
		doorbell, err := ipts.Control.Doorbell()
		if err != nil {
			fmt.Printf("%+v\n", err)
			break
		}

		for doorbell != ipts.Control.CurrentDoorbell() {
			timeout = time.Now().Add(5 * time.Second)

			count, err := ipts.Control.Read(buffer)
			if err != nil {
				fmt.Printf("%+v\n", err)
				return
			}

			if count == 0 {
				continue
			}

			err = IptsDataHandleInput(ipts)
			if err != nil {
				fmt.Printf("%+v\n", err)
				return
			}

			err = ipts.Protocol.Reset()
			if err != nil {
				fmt.Printf("%+v\n", err)
				return
			}

			err = ipts.Control.SendFeedback()
			if err != nil {
				fmt.Printf("%+v\n", err)
				return
			}

			ipts.Control.IncrementDoorbell()
		}

		if time.Now().Before(timeout) {
			time.Sleep(10 * time.Millisecond)
		} else {
			time.Sleep(200 * time.Millisecond)
		}
	}

	err = ipts.Devices.Destroy()
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	err = ipts.Control.Stop()
	if err != nil {
		fmt.Printf("%+v\n", err)
	}
}

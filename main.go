package main

import (
	"fmt"
)

type IptsContext struct {
	Control    *IptsControl
	DeviceInfo *IptsDeviceInfo
	Devices    *IptsDevices
}

func main() {
	ipts := &IptsContext{}
	ipts.Control = &IptsControl{}

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
		ipts.DeviceInfo.Vendor, ipts.DeviceInfo.Device)

	ipts.Devices = &IptsDevices{}

	err = ipts.Devices.Create(ipts.DeviceInfo)
	if err != nil {
		fmt.Printf("%+v\n", err)
		return
	}

	buffer := make([]byte, ipts.DeviceInfo.DataSize)

	for {
		count, err := ipts.Control.Read(buffer)
		if err != nil {
			fmt.Printf("%+v\n", err)
			return
		}

		if count == 0 {
			continue
		}

		err = IptsDataHandleInput(ipts, buffer)
		if err != nil {
			fmt.Printf("%+v\n", err)
			break
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

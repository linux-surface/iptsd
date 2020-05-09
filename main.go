package main

import (
	"bytes"
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
	reader := bytes.NewReader(buffer)

	for {
		count, err := ipts.Control.Read(buffer)
		if err != nil {
			fmt.Printf("%+v\n", err)
			return
		}

		if count == 0 {
			continue
		}

		err = IptsDataHandleInput(ipts, reader)
		if err != nil {
			fmt.Printf("%+v\n", err)
			break
		}

		err = IptsUtilsReset(reader)
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

package main

import (
	"fmt"
)

func main() {
	ipts := &IPTS{}

	ipts.Open()
	defer ipts.Close()

	fmt.Printf("Connected to device %04x:%04x\n",
		ipts.DeviceInfo.Vendor, ipts.DeviceInfo.Device)

	channel := make(chan []byte)
	go IptsDataHandleChannel(ipts, channel)

	ipts.Start()
	defer ipts.Stop()

	buffer := make([]byte, ipts.DeviceInfo.DataSize)

	for {
		if ipts.Read(buffer) == 0 {
			continue
		}

		channel <- buffer
		buffer = make([]byte, ipts.DeviceInfo.DataSize)
	}
}

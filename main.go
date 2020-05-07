package main

import (
	"fmt"
	"time"
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

	timeout := time.Now().Add(5 * time.Second)
	buffer := make([]byte, ipts.DeviceInfo.DataSize)

	for {
		count := ipts.Read(buffer)

		if count > 0 {
			timeout = time.Now().Add(5 * time.Second)
			channel <- buffer
			buffer = make([]byte, ipts.DeviceInfo.DataSize)
			continue
		}

		if timeout.After(time.Now()) {
			time.Sleep(15 * time.Millisecond)
		} else {
			time.Sleep(200 * time.Millisecond)
		}
	}
}

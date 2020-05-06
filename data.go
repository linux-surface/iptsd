package main

import (
	"bytes"
	"fmt"
)

const (
	IPTS_DATA_TYPE_PAYLOAD      = 0
	IPTS_DATA_TYPE_ERROR        = 1
	IPTS_DATA_TYPE_VENDOR_DATA  = 2
	IPTS_DATA_TYPE_HID_REPORT   = 3
	IPTS_DATA_TYPE_GET_FEATURES = 4
)

type IptsData struct {
	Type     uint32
	Size     uint32
	Buffer   uint32
	Reserved [52]uint8
}

func IptsDataHandleInput(ipts *IPTS, data []byte) {
	buffer := bytes.NewReader(data)
	header := IptsData{}

	IptsUtilsRead(buffer, &header)

	fmt.Printf("===============\n")

	for i := uint32(0); i < header.Size; i += 30 {
		n := uint32(30)
		if i+30 >= header.Size {
			n = header.Size - i
		}

		for j := uint32(0); j < n; j++ {
			fmt.Printf("%02x ", data[i+j+64])
		}
		fmt.Printf("\n")
	}

	switch header.Type {
	case IPTS_DATA_TYPE_PAYLOAD:
		IptsPayloadHandleInput(ipts, buffer)
		break
	case IPTS_DATA_TYPE_HID_REPORT:
		IptsHidHandleInput(ipts, buffer)
		break
	}

	fmt.Printf("===============\n")
}

func IptsDataHandleChannel(ipts *IPTS, channel chan []byte) {
	for {
		buffer := <-channel
		IptsDataHandleInput(ipts, buffer)
	}
}

package main

import (
	"bytes"
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

	switch header.Type {
	case IPTS_DATA_TYPE_PAYLOAD:
		IptsPayloadHandleInput(ipts, buffer)
		break
	case IPTS_DATA_TYPE_HID_REPORT:
		IptsHidHandleInput(ipts, buffer)
		break
	}
}

func IptsDataHandleChannel(ipts *IPTS, channel chan []byte) {
	for {
		buffer := <-channel
		IptsDataHandleInput(ipts, buffer)
	}
}

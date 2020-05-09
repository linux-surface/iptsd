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

func IptsDataHandleInput(ipts *IptsContext, data []byte) error {
	buffer := bytes.NewReader(data)
	header := IptsData{}

	err := IptsUtilsRead(buffer, &header)
	if err != nil {
		return err
	}

	switch header.Type {
	case IPTS_DATA_TYPE_PAYLOAD:
		err = IptsPayloadHandleInput(ipts, buffer)
		if err != nil {
			return err
		}
		break
	case IPTS_DATA_TYPE_HID_REPORT:
		err = IptsHidHandleInput(ipts, buffer)
		if err != nil {
			return err
		}
		break
	}

	return nil
}

package main

import (
	. "github.com/linux-surface/iptsd/protocol"
)

func IptsDataHandleInput(ipts *IptsContext) error {
	header, err := ipts.Protocol.ReadData()
	if err != nil {
		return err
	}

	switch header.Type {
	case IPTS_DATA_TYPE_PAYLOAD:
		err = IptsPayloadHandleInput(ipts)
		break
	case IPTS_DATA_TYPE_HID_REPORT:
		err = IptsHidHandleInput(ipts)
		break
	}

	if err != nil {
		return err
	}

	return nil
}

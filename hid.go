package main

import (
	"bytes"
)

/*
 * IPTS on surface gen7 appears to make heavy use of HID reports, unlike
 * previous generations. This file can be used to implement handling for them
 * in the future, seperated from the actual singletouch implementation.
 */
func IptsHidHandleInput(ipts *IptsContext, buffer *bytes.Reader) error {
	id := uint8(0)

	err := IptsUtilsRead(buffer, &id)
	if err != nil {
		return err
	}

	// Make sure that we only handle singletouch inputs.
	// 40 is the report ID of the singletouch device in
	// the generic IPTS HID descriptor
	if id != 0x40 {
		return nil
	}

	err = IptsSingletouchHandleInput(ipts, buffer)
	if err != nil {
		return err
	}

	return nil
}

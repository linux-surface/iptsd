package main

import (
	. "github.com/linux-surface/iptsd/protocol"
)

func IptsPayloadHandleInput(ipts *IptsContext) error {
	payload, err := ipts.Protocol.ReadPayload()
	if err != nil {
		return err
	}

	for i := uint32(0); i < payload.Frames; i++ {
		frame, err := ipts.Protocol.ReadPayloadFrame()
		if err != nil {
			return err
		}

		switch frame.Type {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			err = IptsStylusHandleInput(ipts, frame)
			break
		case IPTS_PAYLOAD_FRAME_TYPE_TOUCH:
			err = IptsTouchHandleInput(ipts, frame)
			break
		default:
			// ignored
			err = ipts.Protocol.Skip(frame.Size)
			break
		}

		if err != nil {
			return err
		}
	}

	return nil
}

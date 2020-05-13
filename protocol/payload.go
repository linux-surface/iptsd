package protocol

const (
	IPTS_PAYLOAD_FRAME_TYPE_STYLUS = 6
	IPTS_PAYLOAD_FRAME_TYPE_TOUCH  = 8

	IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM  = 0x0403
	IPTS_REPORT_TYPE_TOUCH_HEATMAP      = 0x0425
	IPTS_REPORT_TYPE_STYLUS_NO_TILT     = 0x0410
	IPTS_REPORT_TYPE_STYLUS_TILT        = 0x0461
	IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL = 0x0460
)

type IptsPayload struct {
	Counter  uint32
	Frames   uint32
	Reserved [4]uint8
}

type IptsPayloadFrame struct {
	Index    uint16
	Type     uint16
	Size     uint32
	Reserved [8]uint8
}

type IptsReport struct {
	Type uint16
	Size uint16
}

func (proto *IptsProtocol) ReadPayload() (IptsPayload, error) {
	payload := IptsPayload{}
	var err error

	payload.Counter, err = proto.ReadInt()
	if err != nil {
		return payload, err
	}

	payload.Frames, err = proto.ReadInt()
	if err != nil {
		return payload, err
	}

	// We don't parse reserved data
	err = proto.Skip(4)
	if err != nil {
		return payload, err
	}

	return payload, nil
}

func (proto *IptsProtocol) ReadPayloadFrame() (IptsPayloadFrame, error) {
	frame := IptsPayloadFrame{}
	var err error

	frame.Index, err = proto.ReadShort()
	if err != nil {
		return frame, err
	}

	frame.Type, err = proto.ReadShort()
	if err != nil {
		return frame, err
	}

	frame.Size, err = proto.ReadInt()
	if err != nil {
		return frame, err
	}

	// We don't parse reserved data
	err = proto.Skip(8)
	if err != nil {
		return frame, err
	}

	return frame, nil

}

func (proto *IptsProtocol) ReadReport() (IptsReport, error) {
	report := IptsReport{}
	var err error

	report.Type, err = proto.ReadShort()
	if err != nil {
		return report, err
	}

	report.Size, err = proto.ReadShort()
	if err != nil {
		return report, err
	}

	return report, nil
}

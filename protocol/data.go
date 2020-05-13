package protocol

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

func (proto *IptsProtocol) ReadData() (IptsData, error) {
	data := IptsData{}
	var err error

	data.Type, err = proto.ReadInt()
	if err != nil {
		return data, err
	}

	data.Size, err = proto.ReadInt()
	if err != nil {
		return data, err
	}

	data.Buffer, err = proto.ReadInt()
	if err != nil {
		return data, err
	}

	// We don't parse reserved data
	err = proto.Skip(52)
	if err != nil {
		return data, err
	}

	return data, nil
}

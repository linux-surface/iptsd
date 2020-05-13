package protocol

type IptsSingletouchData struct {
	Touch uint8
	X     uint16
	Y     uint16
}

func (proto *IptsProtocol) ReadSingletouchData() (IptsSingletouchData, error) {
	data := IptsSingletouchData{}
	var err error

	data.Touch, err = proto.ReadByte()
	if err != nil {
		return data, err
	}

	data.X, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	data.Y, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	return data, nil
}

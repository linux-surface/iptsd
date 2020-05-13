package protocol

const (
	IPTS_STYLUS_REPORT_MODE_PROX   = 1 << 0
	IPTS_STYLUS_REPORT_MODE_TOUCH  = 1 << 1
	IPTS_STYLUS_REPORT_MODE_BUTTON = 1 << 2
	IPTS_STYLUS_REPORT_MODE_RUBBER = 1 << 3
)

type IptsStylusReport struct {
	Elements uint8
	Reserved [3]uint8
}

type IptsStylusReportSerial struct {
	Elements uint8
	Reserved [3]uint8
	Serial   uint32
}

type IptsStylusData struct {
	Timestamp uint16
	Mode      uint16
	X         uint16
	Y         uint16
	Pressure  uint16
	Altitude  uint16
	Azimuth   uint16
	Reserved  [2]uint8
}

type IptsStylusDataNoTilt struct {
	Reserved  [4]uint8
	Mode      uint8
	X         uint16
	Y         uint16
	Pressure  uint16
	Reserved2 uint8
}

func (proto *IptsProtocol) ReadStylusReport() (IptsStylusReport, error) {
	report := IptsStylusReport{}
	var err error

	report.Elements, err = proto.ReadByte()
	if err != nil {
		return report, err
	}

	// We don't parse reserved data
	err = proto.Skip(3)
	if err != nil {
		return report, err
	}

	return report, nil
}

func (proto *IptsProtocol) ReadStylusReportSerial() (IptsStylusReportSerial, error) {
	report := IptsStylusReportSerial{}
	var err error

	report.Elements, err = proto.ReadByte()
	if err != nil {
		return report, err
	}

	// We don't parse reserved data
	err = proto.Skip(3)
	if err != nil {
		return report, err
	}

	report.Serial, err = proto.ReadInt()
	if err != nil {
		return report, err
	}

	return report, nil
}

func (proto *IptsProtocol) ReadStylusData() (IptsStylusData, error) {
	data := IptsStylusData{}
	var err error

	data.Timestamp, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	data.Mode, err = proto.ReadShort()
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

	data.Pressure, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	data.Altitude, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	data.Azimuth, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	// We don't parse reserved data
	err = proto.Skip(2)
	if err != nil {
		return data, err
	}

	return data, nil
}

func (proto *IptsProtocol) ReadStylusDataNoTilt() (IptsStylusDataNoTilt, error) {
	data := IptsStylusDataNoTilt{}
	var err error

	// We don't parse reserved data
	err = proto.Skip(4)
	if err != nil {
		return data, err
	}

	data.Mode, err = proto.ReadByte()
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

	data.Pressure, err = proto.ReadShort()
	if err != nil {
		return data, err
	}

	// We don't parse reserved data
	err = proto.Skip(1)
	if err != nil {
		return data, err
	}

	return data, nil
}

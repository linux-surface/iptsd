package main

import (
	"bytes"
	"fmt"
	"unsafe"
)

type IptsStylusReportTilt struct {
	Elements uint8
	Reserved [3]uint8
}

type IptsStylusReportSerial struct {
	Elements uint8
	Reserved [3]uint8
	Serial   uint32
}

type IptsStylusReportData struct {
	Timestamp uint16
	Mode      uint16
	X         uint16
	Y         uint16
	Pressure  uint16
	Altitude  uint16
	Azimuth   uint16
	Reserved  uint16
}

type IptsStylusReportDataNoTilt struct {
	Reserved  [4]uint8
	Mode      uint8
	X         uint16
	Y         uint16
	Pressure  uint16
	Reserved2 uint8
}

func IptsStylusHandleData(ipts *IPTS, data IptsStylusReportData) {
	fmt.Printf("======\n")
	fmt.Printf("Timestamp: %d\n", data.Timestamp)
	fmt.Printf("Mode: %d\n", data.Mode)
	fmt.Printf("X: %d\n", data.X)
	fmt.Printf("Y: %d\n", data.Y)
	fmt.Printf("Pressure: %d\n", data.Pressure)
	fmt.Printf("Altitude: %d\n", data.Altitude)
	fmt.Printf("Azimuth: %d\n", data.Azimuth)
	fmt.Printf("======\n")
}

func IptsStylusHandleReportSerial(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportSerial{}

	IptsUtilsRead(buffer, &report)

	// TODO: Track serial number and support multiple styli

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportData{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, data)
	}
}

func IptsStylusHandleReportTilt(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportTilt{}

	IptsUtilsRead(buffer, &report)

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportData{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, data)
	}
}

func IptsStylusHandleReportNoTilt(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsStylusReportSerial{}

	IptsUtilsRead(buffer, &report)

	for i := uint8(0); i < report.Elements; i++ {
		data := IptsStylusReportDataNoTilt{}

		IptsUtilsRead(buffer, &data)
		IptsStylusHandleData(ipts, IptsStylusReportData{
			Mode:      uint16(data.Mode),
			X:         data.X,
			Y:         data.Y,
			Pressure:  data.Pressure * 4,
			Altitude:  0,
			Azimuth:   0,
			Timestamp: 0,
		})
	}
}

func IptsStylusHandleInput(ipts *IPTS, buffer *bytes.Reader, frame IptsPayloadFrame) {
	size := uint32(0)

	for size < frame.Size {
		report := IptsReport{}

		IptsUtilsRead(buffer, &report)
		size += uint32(report.Size) + uint32(unsafe.Sizeof(report))

		switch report.Type {
		case IPTS_REPORT_TYPE_STYLUS_NO_TILT:
			IptsStylusHandleReportNoTilt(ipts, buffer)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT:
			IptsStylusHandleReportTilt(ipts, buffer)
			break
		case IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL:
			IptsStylusHandleReportSerial(ipts, buffer)
			break
		default:
			// ignored
			IptsUtilsSkip(buffer, uint32(report.Size))
			break
		}
	}
}

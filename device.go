package main

type IptsDeviceInfo struct {
	Vendor       uint16
	Device       uint16
	HwRevision   uint32
	FwRevision   uint32
	DataSize     uint32
	FeedbackSize uint32
	Reserved     [24]uint8
}

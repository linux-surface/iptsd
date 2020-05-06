package main

import (
	"bytes"
	"fmt"
)

type IptsSingletouchReport struct {
	Touch uint8
	X     uint16
	Y     uint16
}

func IptsSingletouchHandleInput(ipts *IPTS, buffer *bytes.Reader) {
	report := IptsSingletouchReport{}

	IptsUtilsRead(buffer, &report)

	fmt.Printf("=====\n")
	fmt.Printf("Touch: %d\n", report.Touch)
	fmt.Printf("X: %d\n", report.X)
	fmt.Printf("Y: %d\n", report.Y)
}

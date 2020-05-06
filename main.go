package main

import (
	"os"
)

var (
	IPTS_DEVICE = "/dev/ipts"

	IPTS_UAPI_INFO  = _IOR(0x86, 0x01, 44)
	IPTS_UAPI_START = _IO(0x86, 0x02)
	IPTS_UAPI_STOP  = _IO(0x86, 0x03)
)

func main() {
	file, err := os.OpenFile("/dev/ipts", os.O_RDONLY, 0660)
	if err != nil {
		panic(err)
	}

	defer file.Close()
}

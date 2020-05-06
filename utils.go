package main

import (
	"bytes"
	"encoding/binary"
)

func IptsUtilsRead(buffer *bytes.Reader, object interface{}) {
	err := binary.Read(buffer, binary.LittleEndian, object)
	if err != nil {
		panic(err)
	}
}

func IptsUtilsSkip(buffer *bytes.Reader, count uint32) {
	data := make([]byte, count)
	IptsUtilsRead(buffer, &data)
}

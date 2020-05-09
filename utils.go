package main

import (
	"bytes"
	"encoding/binary"

	"github.com/pkg/errors"
)

func IptsUtilsRead(buffer *bytes.Reader, object interface{}) error {
	err := binary.Read(buffer, binary.LittleEndian, object)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func IptsUtilsSkip(buffer *bytes.Reader, count uint32) error {
	data := make([]byte, count)
	return IptsUtilsRead(buffer, &data)
}

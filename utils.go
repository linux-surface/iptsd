package main

import (
	"bytes"
	"encoding/binary"
	"io"

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
	_, err := buffer.Seek(int64(count), io.SeekCurrent)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func IptsUtilsReset(buffer *bytes.Reader) error {
	_, err := buffer.Seek(0, io.SeekStart)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

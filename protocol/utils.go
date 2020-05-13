package protocol

import (
	"bytes"

	"github.com/pkg/errors"
)

func readInt(buffer *bytes.Buffer, width int) (uint64, error) {
	result := uint64(0)

	for i := 0; i < width; i++ {
		b, err := buffer.ReadByte()
		if err != nil {
			return 0, errors.WithStack(err)
		}

		result = (result << 8) | uint64(b)
	}

	return result, nil
}

func readUint8(buffer *bytes.Buffer) (uint8, error) {
	r, err := readInt(buffer, 1)
	return uint8(r), err
}

func readUint16(buffer *bytes.Buffer) (uint16, error) {
	r, err := readInt(buffer, 2)
	return uint16(r), err
}

func readUint32(buffer *bytes.Buffer) (uint32, error) {
	r, err := readInt(buffer, 4)
	return uint32(r), err
}

package protocol

import (
	"bytes"
	"io"

	"github.com/pkg/errors"
)

type IptsProtocol struct {
	buffer *bytes.Reader
}

func (proto *IptsProtocol) Create(data []byte) {
	proto.buffer = bytes.NewReader(data)
}

func (proto *IptsProtocol) Skip(count uint32) error {
	_, err := proto.buffer.Seek(int64(count), io.SeekCurrent)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (proto *IptsProtocol) Reset() error {
	_, err := proto.buffer.Seek(0, io.SeekStart)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (proto *IptsProtocol) Read(data []byte) error {
	_, err := proto.buffer.Read(data)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (proto *IptsProtocol) ReadByte() (uint8, error) {
	b, err := proto.buffer.ReadByte()
	if err != nil {
		return 0, errors.WithStack(err)
	}

	return uint8(b), nil
}

func (proto *IptsProtocol) ReadShort() (uint16, error) {
	a, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	b, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	c := uint16(a)
	d := uint16(b)

	return (d << 8) | c, nil
}

func (proto *IptsProtocol) ReadInt() (uint32, error) {
	a, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	b, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	c, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	d, err := proto.ReadByte()
	if err != nil {
		return 0, err
	}

	e := uint32(a)
	f := uint32(b)
	g := uint32(c)
	h := uint32(d)

	return (h << 24) | (g << 16) | (f << 8) | e, nil
}

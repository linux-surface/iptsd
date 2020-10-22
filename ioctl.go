package main

import (
	"os"
	"syscall"

	"github.com/pkg/errors"
)

func ioctl(file *os.File, cmd uintptr, arg uintptr) error {
	_, _, err := syscall.Syscall(syscall.SYS_IOCTL, file.Fd(), cmd, arg)
	if err != 0 {
		return errors.WithStack(err)
	}
	return nil
}

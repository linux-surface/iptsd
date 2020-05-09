package main

// #include <linux/ioctl.h>
import "C"
import (
	"os"
	"syscall"

	"github.com/pkg/errors"
)

func _IOC(dir uintptr, typ uintptr, nr uintptr, size uintptr) uintptr {
	return (dir << C._IOC_DIRSHIFT) |
		(typ << C._IOC_TYPESHIFT) |
		(nr << C._IOC_NRSHIFT) |
		(size << C._IOC_SIZESHIFT)
}

func _IO(typ uintptr, nr uintptr) uintptr {
	return _IOC(C._IOC_NONE, typ, nr, 0)
}

func _IOR(typ uintptr, nr uintptr, size uintptr) uintptr {
	return _IOC(C._IOC_READ, typ, nr, size)
}

func _IOW(typ uintptr, nr uintptr, size uintptr) uintptr {
	return _IOC(C._IOC_WRITE, typ, nr, size)
}

func _IOWR(typ uintptr, nr uintptr, size uintptr) uintptr {
	return _IOC(C._IOC_READ|C._IOC_WRITE, typ, nr, size)
}

func ioctl(file *os.File, cmd uintptr, arg uintptr) error {
	_, _, err := syscall.Syscall(syscall.SYS_IOCTL, file.Fd(), cmd, arg)
	if err != 0 {
		return errors.WithStack(err)
	}
	return nil
}

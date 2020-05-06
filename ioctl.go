package main

// #include <linux/ioctl.h>
import "C"
import (
	"os"
	"syscall"
)

func _IOC(dir uint, typ uint, nr uint, size uint) uint {
	return (dir << C._IOC_DIRSHIFT) |
		(typ << C._IOC_TYPESHIFT) |
		(nr << C._IOC_NRSHIFT) |
		(size << C._IOC_SIZESHIFT)
}

func _IO(typ uint, nr uint) uint {
	return _IOC(C._IOC_NONE, typ, nr, 0)
}

func _IOR(typ uint, nr uint, size uint) uint {
	return _IOC(C._IOC_READ, typ, nr, size)
}

func _IOW(typ uint, nr uint, size uint) uint {
	return _IOC(C._IOC_WRITE, typ, nr, size)
}

func _IOWR(typ uint, nr uint, size uint) uint {
	return _IOC(C._IOC_READ|C._IOC_WRITE, typ, nr, size)
}

func ioctl(file *os.File, cmd uintptr, arg uintptr) error {
	_, _, err := syscall.Syscall(syscall.SYS_IOCTL, file.Fd(), cmd, arg)
	if err != 0 {
		return err
	}
	return nil
}

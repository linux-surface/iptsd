package main

import (
	"os"
	"syscall"
	"unsafe"

	"github.com/pkg/errors"
)

type Epoll struct {
	fd int
}

func (epoll *Epoll) Create() error {
	file, err := syscall.EpollCreate1(0)
	if err != nil {
		return errors.WithStack(err)
	}

	epoll.fd = file
	return nil
}

func (epoll *Epoll) Listen(file *os.File, events uint32) error {
	fd := int(file.Fd())
	ev := &syscall.EpollEvent{
		Events: events,
		Fd:     int32(fd),
	}

	err := syscall.EpollCtl(epoll.fd, syscall.EPOLL_CTL_ADD, fd, ev)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (epoll *Epoll) Destroy() error {
	err := syscall.Close(epoll.fd)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}

func (epoll *Epoll) Wait(events []syscall.EpollEvent) error {
	if len(events) == 0 {
		return nil
	}

	wait := -1
	ptr := unsafe.Pointer(&events[0])

	_, _, err := syscall.Syscall6(syscall.SYS_EPOLL_WAIT,
		uintptr(epoll.fd),
		uintptr(ptr),
		uintptr(len(events)),
		uintptr(wait), 0, 0)

	if err != 0 && err != syscall.EINTR {
		return errors.WithStack(err)
	}

	return nil
}

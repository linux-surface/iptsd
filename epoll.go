package main

import (
	"os"
	"syscall"

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
	_, err := syscall.EpollWait(epoll.fd, events, -1)
	if err != nil && err != syscall.EINTR {
		return errors.WithStack(err)
	}

	return nil
}

package main

import (
	"os"
	"syscall"
)

type Epoll struct {
	fd int
}

func (epoll *Epoll) Create() {
	fd, err := syscall.EpollCreate1(0)
	if err != nil {
		panic(err)
	}

	epoll.fd = fd
}

func (epoll *Epoll) Listen(file *os.File, events uint32) {
	fd := int(file.Fd())
	ev := &syscall.EpollEvent{
		Events: events,
		Fd:     int32(fd),
	}

	err := syscall.EpollCtl(epoll.fd, syscall.EPOLL_CTL_ADD, fd, ev)
	if err != nil {
		panic(err)
	}
}

func (epoll *Epoll) Destroy() {
	syscall.Close(epoll.fd)
}

func (epoll *Epoll) Wait(events []syscall.EpollEvent) {
	_, err := syscall.EpollWait(epoll.fd, events, -1)
	if err != nil && err != syscall.EINTR {
		panic(err)
	}
}

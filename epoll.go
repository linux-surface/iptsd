package main

import (
	"os"
	"syscall"
	"unsafe"

	"github.com/pkg/errors"
)

var (
	errEINTR error = syscall.EINTR
	_zero    uintptr
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

/*
 * Modified version of [1], to prevent allocating EINTR repeatedly.
 *
 * [1]: https://github.com/golang/go/blob/master/src/syscall/zsyscall_linux_amd64.go#L1637
 */
func syscallWait(epfd int, events []syscall.EpollEvent, msec int) (int, error) {
	var _p0 unsafe.Pointer

	if len(events) > 0 {
		_p0 = unsafe.Pointer(&events[0])
	} else {
		_p0 = unsafe.Pointer(&_zero)
	}

	r0, _, e1 := syscall.Syscall6(syscall.SYS_EPOLL_WAIT, uintptr(epfd),
		uintptr(_p0), uintptr(len(events)), uintptr(msec), 0, 0)

	n := int(r0)
	if e1 == syscall.EINTR {
		return n, errEINTR
	} else if e1 != 0 {
		return n, e1
	}

	return n, nil
}

func (epoll *Epoll) Wait(events []syscall.EpollEvent) error {
	_, err := syscallWait(epoll.fd, events, -1)
	if err != nil && err != syscall.EINTR {
		return errors.WithStack(err)
	}

	return nil
}

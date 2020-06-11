# Intel Precise Touch & Stylus

This is the userspace part of IPTS (Intel Precise Touch & Stylus) for Linux.

With IPTS the Intel Management Engine acts as an interface for a touch
controller, returning raw capacitive touch data. This data is processed
outside of the ME and then relayed into the HID / input subsystem of the OS.

This daemon relies on a kernel driver that can be found here: 
https://github.com/linux-surface/intel-precise-touch/tree/feature/uapi

The driver will establish and manage the connection to the IPTS hardware. It
will also set up an API that can be used by userspace to read the touch data
from IPTS.

The daemon will connect to the IPTS UAPI and start reading data. It will
parse the data, and generate input events using uinput devices. The reason for
doing this in userspace is that parsing the data requires floating points,
which are not allowed in the kernel.

### What is working?
 * MS Surface Gen 4-6:
   * Stylus Input
   * Multitouch Finger Input
 * MS Surface Gen 7:
   * Singletouch Finger Input
 * HP Spectre 13 x2 (only non-surface device to use IPTS)
   * Entirely untested

### What doesn't work?
 * MS Surface Gen 4-6:
   * Multitouch pressure / contact area calculation
   * Finger tracking can glitch when pressing hard on the display
 * MS Surface Gen 7:
   * Multitouch Input
   * Stylus Input
 * HP Spectre 13 x2
   * Entirely untested

**NOTE:** The multitouch code has not been tested on all devices. It is
very likely that it will still need adjustments to run correctly on some
devices. If you have a device with IPTS and want to test it, feel free to
open an issue or join ##linux-surface on Freenode IRC and get in touch.

Tested Devices:
 * Surface Book 1
 * Surface Book 2 (13" and 15")
 * Surface Pro 5
 * Surface Pro 6

### Building
You need to have a recent Go toolchain installed. It is developed with Go
1.14 but there should be no reason why older releases wouldn't work as well.

Reports and / or patches are welcome!

You also need to have the UAPI version of the IPTS kernel driver installed:
https://github.com/linux-surface/intel-precise-touch/tree/feature/uapi

```bash
$ git clone https://github.com/linux-surface/iptsd
$ cd iptsd
$ go build
$ sudo ./iptsd
```

### Installing
If you want to permanently install the daemon, we provide a systemd service
configuration that you can use. If your distro does not use systemd, you will
have to write your own definition, but it shouldn't be too hard.

Patches with support for other service managers are welcome!

You need to run the steps from "Building" first.

```bash
$ sudo cp iptsd /usr/bin/
$ sudo cp service/iptsd.service /etc/systemd/system/
$ sudo cp -r config/ /usr/share/ipts
$ sudo systemctl enable --now iptsd
```

This assumes that you have the UAPI version of the kernel driver installed
permanently, either by building your own kernel with it included, or by using
DKMS.

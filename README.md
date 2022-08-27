# IPTSD

This is the userspace touch processing daemon for Microsoft Surface devices using Intel Precise Touch technology.

The daemon will read incoming HID reports containing raw capacitive touch data, and create standard input events from it using uinput devices.

The following kernel drivers are supported:
 * ipts
 * ithc
 * spi-hid

At the moment, only systemd based distributions are properly supported. The daemon itself does not depend on the service manager in any ways, but it needs to know which hidraw device corresponds to the touchscreen. The easiest way to do this is with udev and a parameterized service file, which only systemd seems to support.

### Installing

IPTSD is included in the linux-surface repository. This is the recommended way of installing it.

If you want to try out changes that are not yet released, GitHub Actions builds Arch Linux, Debian and Fedora packages for every commit. Go to https://github.com/linux-surface/iptsd/actions, select the last successful build and download the artifact named `<your distro>-latest`.

### Building

To build IPTSD from source, you need to install the following dependencies:

 * A C and a C++ compiler
 * meson
 * ninja
 * CLI11
 * fmt
 * gsl
 * hidrd
 * inih
 * spdlog

To build the debugging tools you need to install a few more dependencies. The debugging tools will be enabled automatically when these are detected:

 * cairomm
 * SDL2

Most of the dependencies can be downloaded and included automatically by meson, should your distribution not include them.

```bash
$ git clone https://github.com/linux-surface/iptsd
$ cd iptsd
$ meson build
$ ninja -C build
```

To run iptsd, you need to determine the ID of the hidraw device of your touchscreen:

```bash
$ sudo dmesg | grep hidraw
```

You can then run iptsd with the device path as a launch argument:

```bash
$ sudo ./build/src/daemon/iptsd /dev/hidrawN
```

Alternatively, you can install the files you just built to the system. After a reboot, iptsd will be automatically started by udev:

```bash
$ sudo ninja -C build install
```

On Fedora (or any other SELinux enabled distribution) you also need to fix the SELinux contexts:

```bash
$ sudo semanage fcontext -a -t systemd_unit_file_t -s system_u /usr/lib/systemd/system/iptsd@.service
$ sudo semanage fcontext -a -t usr_t -s system_u /usr/local/bin/ipts*

$ sudo restorecon -vF /usr/lib/systemd/system/iptsd@.service
$ sudo restorecon -vF /usr/local/bin/ipts*
```

This is only neccessary when using `ninja install`. When you install one of the packages from GitHub Actions, or build your own package, everything will just work.

# IPTSD

This is the userspace touch processing daemon for Microsoft Surface devices using Intel Precise
Touch technology.

The daemon will read incoming HID reports containing raw capacitive touch data, stylus coordinates
and DFT pen measurements, and create standard input events from it using uinput devices.

### Installing

IPTSD is included in the linux-surface repository. This is the recommended way of installing it.

**Important:** Support on Debian based distributions only goes back to Debian 11 / Ubuntu 22.04.

If you want to try out changes that are not yet released, GitHub Actions builds Arch Linux, Debian
and Fedora packages for every commit. You'll need to be signed in to GitHub, then go to
https://github.com/linux-surface/iptsd/actions, select the latest successful workflow and download
the artifact named `<your distro>-latest`.

### Building

To build IPTSD from source, you need to install the following dependencies:

 * A C++ compiler
 * meson
 * ninja
 * CLI11
 * Eigen3
 * fmt
 * inih / INIReader
 * gsl
 * spdlog
 * cmake, because some of our dependencies don't ship pkgconfig files

To build the plotting tools for visualizing data, you need to install a few more dependencies.

 * cairomm
 * SDL2

Most of the dependencies can be downloaded and included automatically by meson, should your
distribution not include them.

```bash
$ git clone https://github.com/linux-surface/iptsd
$ cd iptsd
$ meson setup build
$ ninja -C build
```

To run iptsd, you need to determine the ID of the hidraw device of your touchscreen:

```bash
$ sudo ./etc/iptsd-find-hidraw
```

You can then run iptsd with the device path as a launch argument:

```bash
$ sudo ./build/src/iptsd /dev/hidrawN
```

Alternatively, you can install the files you just built to the system. After a reboot, iptsd will
get started by udev automatically:

```bash
$ sudo ninja -C build install
```

On Fedora (or any other SELinux enabled distribution) you also need to fix the SELinux contexts:

```bash
$ sudo semanage fcontext -a -t systemd_unit_file_t -s system_u /usr/lib/systemd/system/iptsd@.service
$ sudo semanage fcontext -a -t usr_t -s system_u '/usr/local/bin/ipts.*'

$ sudo restorecon -vF /usr/lib/systemd/system/iptsd@.service
$ sudo restorecon -vF /usr/local/bin/ipts*
```

This is only necessary when using `ninja install`. When you install one of the packages from
GitHub Actions, or build your own package, everything will just work.

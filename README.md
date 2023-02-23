# IPTSDaemon

This is the user space IPTS daemon for Surface touch screen process

It is supposed to be used with `BigSurface.kext` to enable touch screen & stylus support on macOS.

Multitouch heatmap & stylus data is sent by the kernel driver to be processed in user space then touch & stylus hid reports are sent back to the kernel.

The code is ported from linux-surface/iptsd

**Warning, processing heatmap will add a non-negligible load on CPU (around 5%) **

Use this under your own consideration!

## Installation Steps
#### 1. Download the latest release version **from Release section**

#### 2. Run `install_daemon.sh` in the zip file you just downloaded and all files needed will be copied to desired locations. 

Normally there will be no output, indicating success. After installation IPTSDaemon will start automatically at boot.

## Uninstallation Step

#### Run `uninstall_daemon.sh`

## Custom Configuration

All possible configurations are in `iptsd.conf`, you can tweak IPTSDaemon according to your needs. After changement, **remember to delete `#` in the front and re-run the installation script to update the file**, or you can directly change that file (`/usr/local/iptsd/iptsd.conf`)

### Enable on screen keyboard on login screen

To enable the on screen keyboard to show up on the login screen you need to change your Accessibility settings in the `System Preferences>Users & Groups>Login Options>Accessibility Options` put a checkbox on the `Accessibility Keyboard`.

### Trouble shooting

- Make sure you are using the latest version of `BigSurface` and `IPTSDaemon`

- Check the output of the script, there should NOT be any errors

- If touchscreen still fails to function, then run `IPTSDaemon` manually to see what is going wrong

  ​	Then create an issue telling me **what is your device**, attaching your **ioreg** and output of `sudo dmesg | grep -E "SurfaceS|SurfaceH|SurfaceM|IntelP"` (remember to install `DebugEnhancer.kext` on macOS 12 and newer)

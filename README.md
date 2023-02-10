# IPTSDaemon

This is the user space IPTS daemon for Surface touch screen process

It is supposed to be used with `BigSurface.kext` to enable touch screen & stylus support on macOS.

Raw touching data is sent by the kernel driver to be processed in user space then touch & stylus hid events are sent back to the kernel.

The code is ported from linux-surface/iptsd and quo/iptsd

**Warning, the processing algorithm is NOT optimised enough yet and it consumes around 10% cpu usage when fingers are detected, 4% or less when stylus is detected**

Use this under your own consideration! Touching process is very energy consuming.

## Installation Steps
#### 1. Download the latest release version

#### 2. Install two `dylib`(`fmt` and `inih`) for IPTSDaemon to run properly.

`Homebrew` is recommended to install them:

- Install [Homebrew](https://brew.sh)

- in `Terminal`, execute `brew install fmt inih`

#### 3. Finally, run the script `install_daemon.sh` in the zip file you just downloaded and all files needed will be copied to desired locations. 

Normally there will be no output, indicating success. After installation IPTSDaemon will start automatically at boot.

If you want to disable touch when palm is detected or stylus is detected or near the stylus, go to config folder and find your device's config, add this:

```
[Touch]
DisableOnPalm = true

[Stylus]
# disable touch when using stylus
DisableTouch = true
# disable touch near the stylus
Cone = true
```

### Enable on screen keyboard on login screen

To enable the on screen keyboard to show up on the login screen you need to change your Accessibility settings in the `System Preferences>Users & Groups>Login Options>Accessibility Options` put a checkbox on the `Accessibility Keyboard`.

### Trouble shooting

- Make sure you are using the latest version of `BigSurface` and `IPTSDaemon`

- Check the output of the script, there should NOT be any

- If touchscreen still fails to function, then run `IPTSDaemon` manually to see what is going wrong

  ​	Then create an issue telling me **what is your device**, attaching your **ioreg** and output of `sudo dmesg | grep -E "SurfaceS|SurfaceH|SurfaceM|IntelP"` (remember to install `DebugEnhancer.kext` on macOS 12 and newer)

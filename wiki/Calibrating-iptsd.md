The palm rejection of iptsd relies on measuring the size and aspect ratio of every contact and then comparing these values to a reference to determine whether the contact is valid or not. The default values will try to cover the finger size of most people, but it is possible (and recommended) to tune these values to better match your fingers. This by extension will also improve palm rejection.

The values that are interesting for calibration are:
 * `SizeMin` (Default: 0.2 cm)
 * `SizeMax` (Default: 2.0 cm)
 * `AspectMin` (Default: 1.0)
 * `AspectMax` (Default: 2.5)

To calibrate iptsd, you need to measure the size of your own fingers and then input those values into the configuration file of the daemon. But before you go away and grab a ruler (or just go away), wait! To make this easier, iptsd includes a tool that will do these measurements for you: `iptsd-calibrate`.

### Calibrate
The first thing you need to do is stop iptsd.

```bash
sudo systemctl stop 'iptsd@*.service'
```

Once that is done you can run the calibration tool.

```bash
sudo iptsd-calibrate $(sudo iptsd-find-hidraw)
```

You will see something like this:

```
[17:46:55.678] [info] Connected to device 045E:0021
[17:46:55.678] [info] Samples: 0
[17:46:55.678] [info] Size:    0.000 (Min: 0.000; Max: 0.000)
[17:46:55.678] [info] Aspect:  0.000 (Min: 0.000; Max: 0.000)
```

The values will update as soon as you touch the display and iptsd registers a contact. You can now start the actual calibration process. Touch the screen with each of your fingers, one time pressing hard and one time pressing lightly. Then put all your fingers on the display at the same time. You should also do some common gestures like pinch-to-zoom or swiping across the display with three or four fingers, since gestures tend to deform the contact. If you only press your fingers straight on the display, iptsd will detect touches fine but will fail to detect gestures.

There is no need for extensively long contacts. Keeping the finger on the display for a long time won't improve calibration accuracy, since you only need a minimal and a maximal value. But it has the potential to introduce noise into the calibration process which will distort the Min/Max values. 3000 to 4000 samples should be plenty.

**NOTE**: During the calibration, palm rejection is not active! So dont put your palm on the display. If you can, detach the display from the keyboard.

**Press `ctrl-C` to end calibration.**

When you are done with the calibration, you should see something like this in the terminal:

```
[17:46:55.678] [info] Connected to device 045E:0021
[17:46:57.186] [info] Samples: 3629
[17:46:57.186] [info] Size:    0.636 (Min: 0.425; Max: 0.859)
[17:46:57.187] [info] Aspect:  1.298 (Min: 1.021; Max: 2.323)
```

In this case the new values would be:
 * `SizeMin`: 0.425 cm
 * `SizeMax`: 0.859 cm
 * `AspectMin`: 1.021
 * `AspectMax`: 2.323

##### using the auto-generated calibration settings
iptsd-calibrate will create 3 configuration files in the current directory with varying tolerances:[1]
```
Three config snippets have been generated for you in the current directory.
Run the displayed to command to install them, and restart iptsd.

Recommended:
    sudo cp iptsd_calib_045E_001F_1793216591_2mm.conf /etc/iptsd.d/91-calibration-045E-001F.conf

If iptsd misses inputs:
    sudo cp iptsd_calib_045E_001F_1793216591_10mm.conf /etc/iptsd.d/91-calibration-045E-001F.conf

For manual finetuning:
    sudo cp iptsd_calib_045E_001F_1793216591_0mm.conf /etc/iptsd.d/91-calibration-045E-001F.conf
```
These can be copied to the configuration directory using the commands your calibration tool outputs.
*The 0mm or 2mm files will likely work best. The 10mm one has very high tolerance and will probably not palm-reject correctly.*

##### or, manually
Create a file `/etc/iptsd.d/90-calibration.conf`[2] with a text editor of your choice, and enter the above values like this:
```ini
[Contacts]
SizeMin = 0.425
SizeMax = 0.859
AspectMin = 1.021
AspectMax = 2.323
```

### Apply calibration settings
After writing a new calibration file, you can restart iptsd to try it out.

```bash
sudo systemctl start $(sudo iptsd-find-service)
```

If you have issues with iptsd not detecting a particular gesture or contact, you should repeat this process and only do the gesture that iptsd has problems with. Then compare the reported values with the ones from your earlier run and adjust them accordingly.


[1]*The prefix number in the `XX-calibration.conf` filename sets the priority of the config file. Higher number = higher priority, the highest priority file will override lower ones. If you don't care and just want one config file, you can just overwrite the existing calibration config file.*
[2]*Check that the configuration file with the highest priority[1] is the configuration file you want applied.*

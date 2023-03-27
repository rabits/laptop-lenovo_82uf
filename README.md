# How to Linux/Ubuntu on Lenovo Yoga i7

Lenovo Yoga i7 16IAH7 - Yoga 7i (16" Intel) with Intel Arc Graphics

* Processor
   * 12th Generation Intel Core i7-12700H Processor (E-cores up to 3.50 GHz P-cores up to 4.70 GHz) 
* Graphic Card
   * Embedded: i7-12700H Intel Iris Xe Graphics
   * Discrette: Intel Arc A370M 4GB GDDR6
* Memory
   * 32 GB LPDDR5-4800MHz (Soldered)
* Storage
   * 1 TB SSD M.2 2242 PCIe Gen4 TLC
* Display
   * 16" WQXGA (2560x1600), IPS, Glare, Touch, 100%sRGB, Low blue light, 400 nits, 60Hz
   * Digital Pen - 4096 pressure levels
* Camera
   * FHD IR/RGB Hybrid with Dual Array Microphone & Privacy Shutter
* Fingerprint Reader
   * Goodix 55b4
* Keyboard
   * White Backlit, Storm Grey - English (US)
* WLAN
   * Wi-Fi 6E 2x2 AX & Bluetooth® 5.1 or above
* Battery
   * LiPo 100W - with optimizations ~14h

## Digital pen

Works fine out of the box, tested in blender

## Fingerprint setup

1. Follow https://gist.github.com/d-k-bo/15e53eab53e2845e97746f5f8661b224 to flash firmware
2. You can build deb package of libfprint with patch in fingerprint dir or use built deb pack
3. Enable for login: https://askubuntu.com/questions/1393550/enabling-fingerprint-login-in-xubuntu

I used to place pam to xfce4-sceensaver and to lightdm configs - otherwise it will be ok for sudo.
Also `max-retries=15` helps with amount of retries.

Unfortunately the scaner still works so-so, but it works.

## Battery / performance

* Slimbook Battery - configure performance profiles
* Slimbook Intel - Set CPU performance TDP

When AC is connected - on balance mode TLP (tlp-stat) will swith to performance

## Audio speakers and mic

By default it's thin, but add those lines to `/etc/modprobe.d/snd.conf` and it will be fine:
```
options snd-sof-intel-hda-common hda_model-alc287-yoga9-bass-spk-pin
```

## Keyboard brightness hotkeys

They are just not registering out of the box, so install Linux kernel >= 6.2 and they will work.

## Screen auto rotation

Use a small python script `screen_rotate.py` laying around to rotate the screen using iio devices.

Just put this script into /usr/local/bin directory, change to executable and add it into xfce's
session and startup since it operates with Xorg and needs it's session.

## Web cam & IR cam

Are available through usual `/dev/video0` & `/dev/video2` devs (add yourself to video group to have
access). Could be used with:
```sh
$ mpv --demuxer-lavf-o=input_format=mjpeg av://v4l2:/dev/video0 --profile=low-latency
```

## Intel embedded & Arc videocard + video decoding

* Arc is well supported since Linux 6.2, so install it.
* Install libraries & utils: https://dgpu-docs.intel.com/installation-guides/ubuntu/ubuntu-jammy-arc.html
* To run any app with Arc GPU - just prepend env var to command like: `$ DRI_PRIME=1 <command>`.

### Switch discrete Arc videocard off/on

It eats tons of power even if it does nothing (on powersave):
* Enabled Arc: ~6h
* Disabled Arc: ~18h

But still quite useful when you want oneAPI for Cycles in Blender or play games. I found that
unbinding the kernel driver is just enough to effectively disable the videocard, but it will eat
~2%/hour during RAM sleep if we will not command acpi to disable ARC GPU power.

Here are steps to acheive the good RAM sleep:

1. Install `acpi-call-dkms` package
2. Add `acpi_call` module into autoload config file: /etc/modules-load.d/modules.conf
3. Manually load the acpi_call kernel module to use it right now: `sudo modprobe acpi_call`

To properly disconnect the videocard you need to tell driver to not use it anymore and tell acpi
to disable it's power.

#### 1. Automated way

You can use the attached `intel_arc_acpi.c` to build the binary, which could have the sticky bit to
simplify off/on/status operations for the regular user. It uses acpi-call module and linux sysfs to
manipulate the driver.

Please check the `intel_arc_acpi.c` header and see how to build, install and use the executable.

You can also add it to autostart with "off" which will be enough to disable the GPU on login, and
later when you will need it - just use "on" parameter to enable it again.

#### 2. Manual way

It's a bit complicated, but look:

1. Unbind/remove device from the PCI tree: `echo 1 | sudo tee /sys/class/drm/card1/device/remove`
2. Tell the acpi to poweroff the device: `echo '\\_SB.PC00.PEG2.POFF' | sudo tee /proc/acpi/call`

Now your ARC GPU card is disconnected and `DRI_PRIME=1 glxinfo | grep 'OpenGL renderer'` will show
you that you using the integrated GPU. To enable it back again - just run the next commands:

1. Poweron the device: `echo '\\_SB.PC00.PEG2.PON' | sudo tee /proc/acpi/call`
2. Rescan PCI bus and enable the device driver: `echo 1 | sudo tee /sys/bus/pci/rescan`

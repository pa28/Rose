# Installing on Raspberry Pi

## Target System Configuration

The target system configuration is a Raspberry Pi with a small
touch screen like the
[SmartyPi](https://smarticase.com/products/smartipi-touch-2)
case without a graphics desktop. The programs may be run on either
a Raspberry Pi with a graphics desktop or Debian base Linux system
with a graphics desktop. These options will be documented later.

## System Setup

For information on how I set up my Raspberry Pi see
[Pi Headless Setup](https://github.com/pa28/Rose/blob/main/docs/PiHeadlessSetup.md).
Even though a SmartyPi case need not be headless, my configuration lacks a 
keyboard and mouse so it is just as easy to treat it as a headless
machine.

## Package Repository

Thanks to the people over at [GemFury](https://gemfury.com/) who offer
free public repositories of all kinds I can offer an easy way to 
install HamChrono and keep updated.

On your Raspberry Pi or x86 Debian based distribution create this file:
`/etc/apt/sources.list.d/ve3ysh.list`

And insert this line:
`deb [trusted=yes] https://apt.fury.io/ve3ysh/ /`

Then perform the following:
```
sudo apt update
sudo apt install rose
```

You may also want to install [hamchrono](https://github.com/pa28/GuiPi)
from another project (which is now deprecated). This program is also
supported by RoseShell.

## Launch on Boot

To configure `systemd` to launch RoseExec and other Rose applications do the
following:

1. Run the command `sudo systemctl start rose`. This should start RoseExec
which will run RoseShell which will display on the screen. If this doesn't
happen something has gone wrong.
1. If RoseShell starts run the command `sudo systemctl enable rose`. This
will enable launch on boot.

To stop auto launch run `sudo systemctl disable rose`. To stop the service
from running without disabling auto launch run `sudo systemctl stop rose`.

You may launch any of the included programs from the command line either
by connecting a keyborad or by using SSH to log into the Pi over the network.
However, the package includes a `systemd`
[service](https://github.com/pa28/Rose/blob/main/resources/systemd/rose.service)
to launch `RoseExec` which will start `RoseShell` from which you may select
other applications. The service file runs the programs as user 1000 which
is the default user and will be `pi` unless you have changed it. You can edit
the service file at `/etc/systemd/system/rose.service` if you want to change
the user (change the 1000 in three palces) or change the program that launches.
If you change the program be aware that RoseExec changes the ownership and
permissions of `/sys/class/backlight/rpi_backlight/brightness` which allows
applications to modify the screen backlight intensity.

If you make changes to the service file you will have to run `sudo systemctl daemon-reload`.

All of the included applications are primitive; the GUI library and applications
are still under development.

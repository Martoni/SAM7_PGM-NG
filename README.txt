This program attempts to download firmware to an Atmel SAM7 chip
by communicating with its embedded boot agent.  Similar to Atmel's
SAM-BA utility, except that this one runs on Linux and is GPL'd.

Current, this code only communicates via serial port to the DBGU port.
You must NOT use USB.  If a USB cable is attached while the boot agent
is running, it will switch into USB mode.

The chip must be running the Atmel boot agent.  For SAM7S chips,
follow the boot agent recovery process.  For SAM7X chips, short the
erase jumper for 1/2 second, then reboot the board to enter the boot
agent.

This code is ALPHA quality.  Only minimal testing has been done.
Only AT91SAM7S64 and AT91SAM7X256 chips have been tested, though
the other SAM7S and SAM7X chips are likely to work.


Contact: paul@pjrc.com




LCD Display showing DNLA/UPnP player status
-------------------------------------------

This little program is a passive UPnP control point that connects to a UPnP/DNLA
renderer (e.g. [gmrender-resurrect][]) anywhere in your
local network.
It listens for changes in the state of the player (Title, Album etc.,
Play/Paused/Stop) and displays it on a common 16x2 LCD display.

### Connect the Hardware

First, we need to connect the LCD display to the Raspberry Pi.
You need
   - One 16x2 LCD display (HD44780 compatible; _very_ common and cheap display,
     less than $3 on eBay)
   - One female 13x1 header connector: one row with 13 contacts to plug into
     one row of the Raspberry Pi GPIO header.
   - Cable and soldering iron (Of course, you can do it the breadboard way
     if you like)

![LCD Display and connector][parts]

First: identify the pins on the LCD display. They typically have 16 solder pins
(sometimes 14 when they don't have a backlight). Pin 1 is usually closer to the
edge of the board. Often marked with a '1' or a dot.

We want to connect them to the outer row of the GPIO connector, which are 13
pins. The GPIO connector _P1_ has two rows, the pins are counted in
zig-zag, so this means that we're connecting to the _even_ pins (P1-02 .. P1-26,
see http://elinux.org/RPi_Low-level_peripherals for reference).

If you put the Raspberry Pi in front of you, with the GPIO pins facing you,
then P1-02 is to your right, P1-26 is to your left.

Connect
   - **LCD 1** _(GND)_ to **GPIO Pin P1-06** (3rd from right, GND)
   - **LCD 2** _(+5V)_ to **GPIO Pin P1-04** (2nd from right, +5V)
     _Note, the first two wires are 'crossed'_
   - **LCD 3** _(contrast)_ to **LCD 1** (GND)
       _the contrast is controllable with a resistor, but connecting it to GND
       is just fine_
   - **LCD 4** _(RS)_ to **GPIO Pin P1-08** (4th from right, Bit 14)
   - **LCD 5** _(R/-W)_ to **LCD 1** _We only write to the display,
      so we set this pin to GND_
   - **LCD 6** _(Clock or Enable)_ to **GPIO Pin P1-12** (6th from right, Bit 18)
   - LCD 7 to LCD 10 are _not connected_
   - **LCD 11** _(Data 4)_ to **GPIO Pin P1-16** (8th from right, Bit 23)
   - **LCD 12** _(Data 5)_ to **GPIO Pin P1-18** (9th from right, Bit 24)
   - **LCD 13** _(Data 6)_ to **GPIO Pin P1-22** (11th from right, Bit 25)
   - **LCD 14** _(Data 7)_ to **GPIO Pin P1-24** (12th from right, Bit 8)

Cross check: With this connection, you should end up with the following
pins not connected:
   - LCD Pin 7, 8, 9, 10 (and 15, 16 if it has these pins)
   - GPIO P1-02, P1-10, P1-14, P1-20, P1-26 (seen from the right, this
     is pin 1, 5, 7, 10, 13).

I would suggest to first connect short cables to all LCD pins that need to be
connected, then connect them right to left to the 13x1 header. The first two
cables end up crossing over, all others are nicely sequenced.

![Put together][soldered]

Now, plug this into the outer row of your Raspberry Pi GPIO:
![Connected][connected]

### Compile the program

Here are the commands you need to execute on your Raspberry Pi shell.

First, you need to have libupnp installed.

    sudo apt-get install libupnp-dev

Get the source. If this is your first time using git, you first need to install
it:

    sudo apt-get install git

.. Then check out the source:

    git clone https://github.com/hzeller/upnp-display.git

Now change into the directory of the checked out source and simply compile it
with `make`:
   
    cd upnp-display
    make


### Start the program

You need to start the program as root, as it needs to access the GPIO pins:

    sudo ./upnp-display

The LCD display should now print that it is waiting for any renderer;
once it found a renderer, it will display the title/album playing.

If you have multiple renderers in your network, you can select the particular
one you're interested in with the `-n` option:

    sudo ./upnp-display -n "Living Room"

Now, if you use your entertainment system the usual way, this display
shows what currently is played. You can deploy this multiple times
in the same network, so you can have one display in every room :)

![yay, working][in-operation]

### Compatibility

#### UPnP Renderers
This should work with all renderers, that do proper eventing of variable
changes. This program does not, at this time, actively query the renderer
but expects it to transmit changes according to the UPnP eventing standard.

Right now, this is tested with [gmrender-resurrect][], which works perfectly.

#### Special characters
While the titles and album names can contain the full UTF-8 characters set,
the LCD displays can't show them by default. So right now, only ASCII characters
are displayed well.

The LCD displays allow for adding custom character bitmaps so this _could_ be
implemented with some effort, but isn't at this time.

#### LCD Displays
We can't check the 'busy'-status of the LCD display, as we can't read from
the interface (the LCD operats at 5V and the GPIO pins only tolerate 3.3V). Hence
the handshake to the display is ensured by waiting after each write which should
be enough according to the data sheet. If you need tweaking, look at
lcd-display.cc

#### Other machines than Raspberry Pi
If you want to connect the display on some other computer
than the Paspberry Pi, you don't have GPIO pins. You have to change the hardware
interfacing and modify lcd-display.cc

[parts]: https://github.com/hzeller/upnp-display/raw/master/images/basic-connector-small.jpg
[soldered]: https://github.com/hzeller/upnp-display/raw/master/images/soldered-small.jpg
[connected]: https://github.com/hzeller/upnp-display/raw/master/images/outer-gpio-row-small.jpg
[in-operation]: https://github.com/hzeller/upnp-display/raw/master/images/in-operation-small.jpg
[gmrender-resurrect]: http://github.com/hzeller/gmrender-resurrect

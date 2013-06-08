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
configuration
   - 8 wires connect LCD display with header.
   - 2 wires connecting LCD pins to the first LCD pin (GND).
   - Not connected LCD: Pin 7, 8, 9, 10 (and 15, 16 if it has these pins)
   - Not connected RPi: GPIO P1-02, P1-10, P1-14, P1-20, P1-26 (seen from the
     right, this is pin 1, 5, 7, 10, 13).

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

#### Unicode support
These LCD displays only support the ASCII character set which is a bit
limited for international titles or artist names.

Luckily, these displays have a way to have up to 8 user defined characters. We
are using this feature to upload a font for characters outside the ASCII range
(uses the excellent [Public Domain fixed Unicode font][ucs-fixed] maintained by
Markus Kuhn).

This works of course only well if there are not more than 8 different non-ASCII
characters on the screen - if you have song titles that are all outside this
range (e.g. your language uses an entirely different script), then this is likely
to fail.

Here you see an example that uses the non-ASCII characters
&auml;, &uuml; and &szlig;

![UTF-8 display][utf-8-display]

(If there is enough demand, I can separate out the unicode-aware display writing
into a separate libray).

#### LCD Displays
Most displays you can get are HD44780 compatible; There are as well
24x2 and 40x2 displays available (also pretty cheap). I found that 16
characters is a bit on the low side to display a useful amount without constant
scrolling. If you get another display, change the #define LCD_DISPLAY_WIDTH in
upnp-display.cc. Usually, the pin-out looks a bit different (2 rows with 7 or 8
lines), but typically it has the same data lines on the same pin numbers - check
your data sheet.

Here a 40 character display from ebay (DMC-50037N)
![40 character display][display-40-char]

#### Other machines than Raspberry Pi
If you want to connect the display on some other computer
than the Paspberry Pi, you don't have GPIO pins. You have to change the hardware
interfacing and modify lcd-display.cc

[parts]: https://github.com/hzeller/upnp-display/raw/master/images/basic-connector-small.jpg
[soldered]: https://github.com/hzeller/upnp-display/raw/master/images/soldered-small.jpg
[connected]: https://github.com/hzeller/upnp-display/raw/master/images/plugged-in-small.jpg
[in-operation]: https://github.com/hzeller/upnp-display/raw/master/images/in-operation-small.jpg
[utf-8-display]: https://github.com/hzeller/upnp-display/raw/master/images/utf8-lcd-small.jpg
[display-40-char]: https://github.com/hzeller/upnp-display/raw/master/images/display-40-char-small.jpg
[gmrender-resurrect]: http://github.com/hzeller/gmrender-resurrect
[ucs-fixed]: http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html

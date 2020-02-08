# Remote Fader Firmware for the Datasat AP20 Digital Cinema Audio Processor

## Why is this here?

_Skip if you don't care about fluff._

Well, easy. During my days in university I joined the student cinema group.
As it happened in those days I was able to contribute to the major task of
replacing the outdated analog projection equipment in our theatre with more
up-to-date equipment. Along with a digital projector and some other equipment
we acquired a new Datasat AP20 audio processor.

Long story short, we needed a simple rotary knob _inside_ the cinema theatre for
the supervisor to control the overall volume during a film show while the actual
audio processor sits upstairs in the projector room. Buying the knob from
Datasat was not an option simply because of the hefty price tag.

But we found that Datasat implements a rather simple ASCII based protocol to
handle a whole bunch of settings on their devices and the AP20 has several
interfaces we could use with RS232 being among those. So we traced a cable
through the building...

The first approach was a TI MSP430 Launchpad with an Alps rotary encoder knob
connected that single-directionally transmits ASCII commands to the AP20. But
would't it be cool to have something with also a display?

## What is this?

The coolest type of display we could come up with for a volume control knob to
be used in a dark room is a Nixie tube display.

The PCB in this project connects two digits of IN12b Nixie tubes to an
MSP430G2553. There are two galvanically isolated voltage domains to protect the 
expensive AP20 against surges from the make-shift high voltage supply of the
Nixies.

The application code mainly does two things:
1. Whenever the knob is turned a volume command 'FADER +x\r' is transmitted
   towards the AP20 via UART. That, in turn, causes the AP20 to respond...
2. ... with a 'FADER xx\r' string denominating the current volume level.
   The number is extracted and displayed on the nixies.

Due to poor (but for now, uncorrectable) PCB design choices there must be some
additional glue and safety around the I2C connection between the MSP and the
MAX GPIO expander IC on the other side of the voltage safety barrier.

## Building and Contributing

This is not tested yet and sort of a long running project of mine.
I am using TI Code Composer Studio 8.3 and TI MSP compiler v18.1.5 on Windows
and the programmer-part of the MSP430 Launchpad for debugging.

Documentation for the AP20 and other helpful stuff: \\
http://www.film-tech.com \\
http://www.film-tech.com/warehouse/manuals/TM-H392_V1.2_AP20.pdf

# USBTypewriterP2S
Code for Parallel to Serial Control Panel

This is the firmware for the USB Typewriter project.  It runs on the USB Typewriter circuitry (hardware version 8.X and above), which utilize an ATMEGA32u4 microcontroller. Earlier versions of the USB Typewriter circuitry used an ATMEGA 328 microcontroller and ran on a different firmware entirely.

The basic principle of operation is this:
Every few ms, the micro detects keypresses by polling a chain of parallel-to-serial (P2S) converters, each pin of which is connected to a gold-plated contact positioned under the typewriter's keys.  When a key is pressed, it connects with and grounds one of these contacts, producing a zero on the P2S converter's input. This zero is locked into the P2S chip's internal register whenever the Parallel-Load (PL) pin of the P2S is clocked. Then, the SCK pin of the P2S is toggled repeatedly to clock the locked-in sensor data (SDA) into the microcontroller.  The clocking of the PL pin is done in an interesting way: to reduce wiring from the microcontroller circuit board to the P2S sensor circuit board, the microcontroller does not have direct control over the PL line.  Instead, there is a circuit on the P2S circuit board that automatically clocks the PL pin whenever the SCK pin is held high or low for several ms.  (When the SCK is in use, the PL pin does not change state.)  This way, only two connections are needed between the microcontroller circuitry and the sensor circuitry (SCK and SDA).

Crucially, one of the P2S pins is not connected to a gold-plated contact, but instead is connected to a hall-effect sensor.  This sensor is used to detect the physical position of the sensor array.  Each pressed key will move the typewriter's spring-loaded crossbar, which the sensor circuitry is glued to. Therefore we can tell how far down a key has been pressed by detecting the physical position of the sensor array as it moves up in down in response to each keypress.  When the hall effect sensor detects that the sensor array has been displaced significantly by a keypress, it will change state.  The software can read this state at the same time it reads in the state of the gold contacts, and it can choose to ignore all keypresses unless the hall-effect sensor reports that the key sensor has been significantly depressed.  This system prevents inadvertant light keypresses from registering.  As of this writing, the Do-it-yourself kit does not include this optional hall-effect-sensor system.

After reading in the state of the P2S sensor array, some debouncing occurs, and then the key pressed is paired with either a USB keycode or an ASCII character.  This information is then sent over USB or Bluetooth, or it is saved to a micro-SD card.  When it is in USB mode, the circuitry also behaves as an SD-card reader, so users can access the contents of the SD card.

Four reed switches are also available, to sense shift, spacebar, backspace, and enter.  There are also hard-wired CTRL, ALT, and CMD buttons on the main microcontroller circuit board.


The project makes use of the LUFA USB library and the FATFs SD card library.  It also provides the option of interfacing with either the Microchip RN42 Bluetooth Module or the EHONG EH-MA41 Bluetooth Module.

Further details on the project can be obtained from the author, Jack Zylkin:
jack@usbtypewriter.com
www.usbtypewriter.com



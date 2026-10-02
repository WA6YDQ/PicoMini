# PicoMini
Turning a Pi Pico into a multi user minicomputer

This project uses a Pi Pico 2 as a multi user computer with segmented ram
for each user (a block of 64K), 3 user serial ports (USB, UART0 and UART1) expandable
to 7 with additional hardware, a custom cpu (currently using the 8080 instruction
set, but can be any of your choosing), multi-level interrupts, an 8 bit I/O bus for
external peripherals and control lines for I/O read and write.

I added an SPI SD card for a hard drive as well as a PSRAM chip (8 MB) for 
future expansion. This could be used to add more users, but context switching 
would be slow doing this.

Each user gets their own protected block of 64K (a full blown 8080 computer system)
and a 20 msec time slice for processing. With 5 users there is no noticeable lag. 

I used a pi pico1 as 2 UARTS to go from 3 users to 5 users. The Pico1 also acts
as 3 ADC ports and an external interrupt controller. The 8 bit bus is used for
communication between the two Picos.

The Pico2 code acts like an operating system. I use 8080 in and out instructions 
as system calls to access the SD card, PSRAM and other I/O devices. Also, I added
2 push button switches as a RESET switch and an ABORT switch. The RESET
switch resets the processor CPU (NOT the pico2) and the ABORT button stops processor
execution allowing the user on the USB serial port to interact with the processor
and memory, load and save memory blocks, etc. Exiting the abort routine returns control
to the processor as if nothing happened.

There are 3 status led's: power (from the Pico's 3.3v line, CPU HALT which lights when
the CPU stops running (ie abort mode) and CPU FAULT (when a bad or unknown opcode
is executed caused by a faulty program).

You will need a Pi Pico V2 (V2 only - the Pico 1 doesn't have the needed ram).

I wrote an operating system in 8080 assembler with hooks to interface with the SDCARD
and other hardware.

You will need the no-OS-FatFS-SD-SDIO-SPI-RPi-Pico library from https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/tree/main

I included my 8080 assembler program (written in C) but you can use your own. This is in 
the asm sub directory. It compiles with make on a linux system. I haven't tried, but
it should work on a windows or mac os system as well.

Included in the asmcode sub directory is an operating system for the computer (boot.hex, boot.asm)
and various diagnostic and test routines.

Defines in the emulator_1.c code set system parameters. The source code is currently 
one very long single file, but I'll likely split it into smaller routines later.


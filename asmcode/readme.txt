The programs in this directory include the 8080 operating system
(boot.asm, boot.com, boot.hex, boot.prn), a line editor (ed.asm
and associated files), a program to test the PSRAM chip (ramtest.asm)
and misc test files.

The program ed.asm is a line editor. Type 'h' <enter> for command list.
ed.asm will need to be compiled:
ASM80 ed.asm 
(using the supplied ASM80 assembler or use your own)
Then copy ed.com to your SD card. Type 'ed' <enter> 
at the OS1> prompt to invoke.


When we build the project, we likely want to validate that what we have is correct.  Before we actually flash, there are some tests we can perform to check the integrity of the results.

First, if we find the ELF binary that was build for us, we can run objdump --headers over it.  This will produce results similar to the following:

    Sections:
    Idx Name          Size      VMA       LMA       File off  Algn
      0 .data         0000034c  3ffe8000  3ffe8000  000000e0  2**4
                      CONTENTS, ALLOC, LOAD, DATA
      1 .rodata       00000060  3ffe8350  3ffe8350  00000430  2**3
                      CONTENTS, ALLOC, LOAD, READONLY, DATA
      2 .bss          00006928  3ffe83b0  3ffe83b0  00000490  2**4
                      ALLOC
      3 .text         000061be  40100000  40100000  00000490  2**2
                      CONTENTS, ALLOC, LOAD, READONLY, CODE
      4 .irom0.text   00029536  40210000  40210000  00006650  2**4
                      CONTENTS, ALLOC, LOAD, READONLY, CODE

Note specifically the location of the `.irom0.text` address location which places it at the correct address space.
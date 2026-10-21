# Fail-Safe Firmware Update Bootloader

A portable C model of a dual-bank update controller. UART transport and STM32 flash I/O are represented by callbacks so the transaction logic can be tested on a host. A candidate image is staged in the inactive bank, checked with CRC32 and a caller-supplied signature verifier, booted as a trial, then committed only after the application reports healthy. A watchdog reset before confirmation rolls back to the previous bank.

```sh
gcc -std=c11 -Wall -Wextra -Werror bootloader.c test_bootloader.c -o boot_test
./boot_test
```

The test verifier models signature acceptance/rejection; it is **not** a cryptographic implementation. An STM32 deployment must supply a public-key verification routine, flash partition/erase/write functions, UART framing, reset-cause and watchdog drivers, and durable boot flags.


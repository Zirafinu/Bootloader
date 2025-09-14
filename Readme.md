# The Bootloader

This is a Bootloader skeleton project.
It illustrates how to implement a simple bootloader, with all core features required.

# The Features

What can this small pice of code possibly do?

- Check the Application for integrity
- Communicate with the Application
- Replace the Application with a Update-Application
- Decrypt an AES-128 stream
- Uncompress a gzip file
- Version info structures for the application and the bootloader
- Minimal testing

## Integrity checks

The supported integrity check is crc-32.
It's tuned for size without sacrificing speed entirely.
Hardware accelerated should be favored.

## Communication

The Bootloader can start into three states:

- The Application, even though valid, requests an update.
- The Application doesn't want to run and actions should be taken after main returns.
- The Application didn't try to communicate and is launched, if valid.

## Updating

From where and how data is copied is entirely up the you.
Try to enable all the Hardware only when it's needed.
Check the example for some guidance.

## AES Decryption

AES decryption is handrolled, thus handle with care and avoid if posisble.

## GZIP

It's practically the [puff](https://github.com/madler/zlib/tree/develop/contrib/puff) implementation.
It features read and write callbacks, to allow for extended computations when in need.

## Version Info

The bootloader and application can use the version info struct, to avoid downgrading and incompatible hardware.

## Testing

It's minimal, i promise :-P
In the example tests can be executed in a qemu vm or on target.
When run on the target the tests are linked into RAM, so that Flash doesn't forfeit after a weeks worth of tests.

# How to Build

To run the "tests" for the default implementations run the following command and see where it gets you:

```bash
cmake --workflow --preset default
```

To get something remotely useful venture into the examples/stm32f407 directory.

There is a small teaser, implementing a Bootloader using the internal Flash.
It features test execution in both qemu-arm-systems and on target via openocd.
Check the CMakePresets.json there for the workflows.
To build the bootloader or blinky application there a key.bin file has to be created containing the AES key.

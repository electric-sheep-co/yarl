# yarl
Yet Another Redis Library

A couple [dev](https://www.nordicsemi.com/?sc_itemid=%7BF2C2DBF4-4D5C-4EAD-9F3D-CFD0276B300B%7D)
[kits](http://cloudconnectkits.org/product/azure-sphere-starter-kit) arrived 
that wanted for a POSIX C Redis library, so this
is a port of [arduino-redis](http://arduino-redis.com) into straight-C. 
Some day, that library will consume this one & morph into just a nice C++ facade.

Additionally, I felt a simple, "bring your own file descriptor" interface 
would do nicely on the simple MCU platforms to be targetted.

To that end, `RedisConnection_t` is `typedef`ed simply to `int`, a.k.a. a
file descriptor. It could be a socket FD, a pipe FD, even an actual file descriptor
if you're so inclined. Anything that can be `read(2)` from and `write(2)`en to. BYOFD!

## Building

The test app:
```
clang -o sphere  -Wall -Werror -I./src -O0 -g ./src/*.c ./test/test.c
```
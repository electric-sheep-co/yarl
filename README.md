# yarl
Yet Another Redis Library

A couple [dev](https://www.nordicsemi.com/?sc_itemid=%7BF2C2DBF4-4D5C-4EAD-9F3D-CFD0276B300B%7D)
[kits](http://cloudconnectkits.org/product/azure-sphere-starter-kit) arrived 
that wanted for a POSIX C Redis library, so this is a port of [arduino-redis](http://arduino-redis.com) into straight-C. 
Some day, that library will consume this one & morph into just a nice C++ facade.

Given the simple MCU platforms being targetted, a "bring your own file descriptor" interface 
seemed like it would do nicely. To that end, `RedisConnection_t` is `typedef`ed simply to `int`, a.k.a. a
file descriptor. It could be a socket FD, a pipe FD, even an actual file descriptor
if you're so inclined. Anything that can be [`read(2)`](http://man7.org/linux/man-pages/man2/read.2.html) 
from and [`write(2)`](http://man7.org/linux/man-pages/man2/write.2.html)-en to. BYOFD!

## Building

### The test app
Should build & run on any POSIX platform.

```
clang -o yarl_test  -Wall -Werror -I./src ./src/*.c ./test/test.c
```

* add `-DDEBUG=1` to generate debugging logging
* add `-O0 -g2` to disable optimizations and generate symbols, to enable debugging via `lldb`/`gdb`

### The Azure Sphere library

Requires the [Azure Sphere SDK](https://docs.microsoft.com/en-us/azure-sphere/app-development/development-environment) (so also Windows & VS17 or VS19).

The VS project in the `azuresphere` directory is preconfigured to build a static library named `libyarl.a` in the standard build output location.

See [this project](https://github.com/rpj/spheremon) for an example of consuming yarl as a build dependency.    

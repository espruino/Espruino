Libs
====

Here you can author libraries for the Espruino with native code.

For more information on the actual build process, see [the build process page](../README_BuildProcess.md)

To get you started, here are some steps that can be made to enable `Hello.world()` in Espruino. 
Of course you have guessed that it will print "Hello World!" to the console. Let's get on with it!

## Create a directory to contain your library files
We'll create a new folder `libs/hello`. In this directory, we'll create our .h and .c files.

## Create the `jswrap_hello.h` and `jswrap_hello.c` files

jswrap_hello.h
```c
void jswrap_hello_world();
```

jswrap_hello.c

```c
#include "jswrap_hello.h"  // We need the declaration of the jswrap_hello_world function
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

// Let's define the JavaScript class that will contain our `world()` method. We'll call it `Hello`
/*JSON{
  "type" : "class",
  "class" : "Hello"
}*/

// Now, we define the `jswrap_hello_world` to be a `staticmethod` on the `Hello` class
/*JSON{
  "type" : "staticmethod",
  "class" : "Hello",
  "name" : "world",
  "generate" : "jswrap_hello_world"
}*/
void jswrap_hello_world() {
    jsiConsolePrint("Hello World!\r\n");
}
```

You can add more files here if you need. It's up to you!

## Modify the `Makefile`

Go to line [606](https://github.com/espruino/Espruino/blob/master/Makefile#L606) and append

```make
INCLUDE += -I$(ROOT)/libs/hello
WRAPPERSOURCES += libs/hello/jswrap_hello.c #you can add more files here if your library depend on them
```

If you want to make a pull request for your new library you'll need to make a `ifdef` guard for it, 
and specify which platforms should have access to your library.
To see it in action, follow the `USE_TRIGGER` definition.

## Compile and test!

First run `make`, now you can run `./espruino` and test your new `Hello.world()` command.

```sh
>Hello.world()
Hello World!
undefined
```

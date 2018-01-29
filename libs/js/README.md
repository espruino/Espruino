Built-in JS modules
=====================

Simply stick the JS modules you want in this directory,
then modify the `boards/BOARDNAME.py` file so it includes
a line that sets `JSMODULESOURCES`:

```
info = {
 ....
 'build' : {
   ....
   'makefile' : [
     ....
     'JSMODULESOURCES+=libs/js/YOUR_MODULE.min.js',
   ]
 }
};
```

or add them as prefix to make

```
JSMODULESOURCES=libs/js/YOUR_MODULE.min.js make
``` 

Then when you rebuild, `require("YOUR_MODULE")` will
magically pull in your module.


Notes
-----

* Until referenced, modules use NO RAM
* Once referenced, the JS code in a module will be executed (from Flash).
* Every function declared in the module's root scope will take up some RAM
* However the code inside that function will be kept in Flash memory, so won't take up RAM.

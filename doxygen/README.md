Doxygen
------


Espruino's functions are tagged such that documentation can be built from them:

```
/// Comment before
void function();

/** Big comment before */
void function();

void function(); ///< Comment after 
```

To build. go to the root of the Espruino project, install [Doxygen](https://www.stack.nl/~dimitri/doxygen), then run:@

```
doxygen Doxyfile
```

Most functions you care about will be in [doxygen/html/globals_0x6a.html](html/globals_0x6a.html)

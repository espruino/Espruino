Contributing
===========

Thanks for thinking about contributing to Espruino! Anything you can add is hugely appreciated, but please can you follow a few simple rules:

* Keep the same coding style
* Ensure that you are not contributing someone else's code, and that you are willing to add your code under Espruino's MPL Licence
* Make sure that what you do doesn't break the Espruino board or the other boards we build for. We can't check all the boards for every commit, so if you break something you'll annoy a whole bunch of people.
* Be aware that Espruino is designed for Microcontrollers - with very low amounts of flash and memory. Both are at a premium so don't statically allocate variables or do other stuff that will use up memory.
* Avoid randomly refactoring everything or renaming things to your own personal style
* Don't add a whole bunch of indirection/abstraction for the sake of it - it'll probably just use of more of our precious memory.
* If you add a new API, try and make it familiar to Arduino/JavaScript users.

Target Areas
-----------

We'll keep the outstanding issues in our bug tracker, but general stuff that would really help us is:

* Tests. If something doesn't work, please make a test for it. Even if you don't fix it it'll help others greatly.
* Documentation. Improving the documentation (especially the auto-generated 'Espruino Reference') would be fantastic.
* Duplication. If the same code is used for multiple platforms, try and make sure it's shared, not duplicated.
* Remove hard-coded stuff. Various things like the SPI filesystem are still hard-coded with ifdefs for each board - we want all that stuff to be generated from build_platform_info.py
* Speed. There are a few areas this could be improved - but please benchmark what you're doing both before and afterwards on the Espruino board to check that what you've done helps
* Memory Usage. Both RAM and Flash are at a premium. Ways of reducing this and making usage more efficient are really appreciated.
* JavaScript compliance (without affecting speed or memory usage too much).

Contributing
-----------
* Please RUN THE TESTS to check that there are no regressions
* Issue us a pull request to [www.github.com/espruino] via GitHub
* Please keep each request small (just include one fix per request)

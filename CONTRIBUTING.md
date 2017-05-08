Contributing
=============

Thanks for thinking about contributing to Espruino! Anything you can add is hugely appreciated, but please can you follow a few simple rules:


Support / Bugs
--------------

First, please try and check that your problem hasn't [already been found](https://github.com/espruino/Espruino/issues) or [covered on our forum](http://www.espruino.com/Forum).

**Do not post 'it doesn't work for me' / 'I can't connect' / 'How do I make a foobar' issues.** Please ask these on the [forum](http://www.espruino.com/Forum) where you will get a much more helpful response. Issues posted here should be for actual software bugs or feature requests.

[Submit bugs](https://github.com/espruino/Espruino/issues) with clear steps to reproduce them: a minimal test case, and an actual and expected result. If you can't come up with these, please [post on the forum](http://www.espruino.com/Forum) first as it may just be something in your code that we can help out with.

Work on Espruino is supported by [sales of our boards](http://www.espruino.com/Order).

**If your board isn't made by us but came pre-installed with Espruino then you should contact the manufacturers.**

We try and support users of the boards we sell, but if you bought a non-official board your issue may not get addressed. In this case, please consider [donating](http://www.espruino.com/Donate) to help cover the time it takes to fix problems (even so, we can't guarantee to fix every problem).


Documentation
-------------

Improvements to Documentation are amazingly helpful, and are very rare so hugely appreciated.

### Functions/Variables

If you want to change something in a [built-in function's documentation](http://www.espruino.com/Reference), look at the heading for the function on that page and there will be a right-arrow (⇒). 

If you click on that it will bring you to the area of Espruino's source code where the function *and the documentation for it* are stored. You can then edit the documentation in that file (above the function) on GitHub and issue a pull request - it's in Markdown format.

### Tutorials

Please [see here](http://www.espruino.com/Writing+Tutorials)

### Other

If there's something specific to an area of the Espruino interpreter *that can't be put in a source file*, please create a Markdown (`.md`) file in the relevant area, and make sure you link to it from other relevant files so it can be found.


Contributing Code
-----------------

### Modules

Please [see here](http://www.espruino.com/Writing+Modules)

### Espruino code

* Please keep the same coding style (See **Coding Style** below)
* Keep the minimum amount of changes in each pull request/commit (this really helps with debugging later on)
* Try not to commit huge whitespace/refactoring changes along with your fix
* Avoid adding newlines, spaces, refactoring everything or renaming things to your own personal style (some things really could do with renaming, but please check first or we may reject your pull request)
* Ensure that you are not contributing someone else's code, and that you are willing to add your code under Espruino's MPL Licence
* Make sure that what you do doesn't break the Espruino board or the other boards we build for. We can't check all the boards for every commit, so if you break something you'll annoy a whole bunch of people.
* Be aware that Espruino is designed for Microcontrollers - with very low amounts of flash and memory. Both are at a premium so don't statically allocate variables or do other stuff that will use up RAM.
* Don't add a whole bunch of indirection/abstraction for the sake of it - it'll probably just use of more of our precious memory.
* If you add a new API, try and make it familiar to Arduino/JavaScript users.
* Please [RUN THE TESTS](tests/README.md) before and after your changes to check that there are no regressions
* Finally, please issue us a pull request to [www.github.com/espruino](https://www.github.com/espruino/Espruino) via GitHub. It's way easier for us to incorporate, give credit, and track changes that way.


Target Areas
-----------

We'll keep the outstanding issues in [GitHub's issue list](https://github.com/espruino/Espruino/issues), but general stuff that would really help us is:

* **Tests** If something doesn't work, please make a test for it. Even if you don't fix it it'll help others greatly. Bonus points if it's in a pull request :)
* **Documentation** Improving the documentation (either the [EspruinoDocs](https://github.com/espruino/EspruinoDocs) project, source code, the auto-generated reference, or the Markdown files in this project) would be fantastic.
* **Duplication** If the same code is used for multiple platforms, try and make sure it's shared, not duplicated.
* **Remove hard-coded stuff** Some things are still hard-coded with ifdefs for each board - we want all that stuff to be generated from `build_platform_info.py` using the board definition file.
* **Speed** There are a few areas this could be improved - but please benchmark what you're doing both before and afterwards on the Espruino board to check that what you've done helps.
* **Memory Usage** Both RAM and Flash are at a premium. Ways of reducing this (including stack usage) and making usage more efficient are really appreciated.
* **JavaScript compliance** - without affecting speed or memory usage too much


Coding Style
-----------

The rough coding style is as follows, but you should get a good idea from the code. If we've missed anything obvious please let us know!

* Unix file format (`CR`, not `CR LF` for newlines)
* 2 spaces for indents
* Open curly braces on the same line
* No Tabs used, only spaces
* Use `bool` for booleans - not `int`
* ```//``` comments for single lines, ```/* ... */``` for multiple lines
* Half-hearted Doxygen compatibility: use ```///<``` for function/variable declaration documentation (if on same line), and ```/** ... */``` if doing it right before a function
* If you're adding comments to a function, make sure they're in the header file, and if you are going to add comments in the source, make sure the two match.
* Do not add `setFooBar // This sets the Foo to Bar` style comments to functions/variables. Only add comments if they *add something that is not obvious from the declaration itself*
* Use new lines in code sparingly (only where it really makes sense)



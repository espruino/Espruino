In order to build this project, you will need a development environment.  We recommend the [Eclipse Mars](http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/marsr) release with the C Developer Tools (CDT) perspective.  In addition to the base Eclipse, the following plugins are recommended (but not required):

* [Log Viewer](https://marketplace.eclipse.org/content/logviewer) - Tail tool inside Eclipse.  Good for watching the CDT build logs.
* [Path Tools](https://marketplace.eclipse.org/content/path-tools) - Working with file paths within the Eclipse environment.

The project can also be built from the command line but we recommend Eclipse because of its integrated tooling allow us to run makes, see the output of the makes and see the C source files annotated with the errors produced during a build (assuming that there are any).

We need an Xtensa version of GCC.  The version that has been tested is Xtensa GCC v.5.10.  This was obtained here:

[http://www.esp8266.com/viewtopic.php?f=9&t=820](http://www.esp8266.com/viewtopic.php?f=9&t=820)

We will also need a copy of python.  This can be obtained here:

[https://www.python.org/downloads/](https://www.python.org/downloads/)

We can download the [Python](https://www.python.org/downloads/) 3.4.3 (or better).  This will create a `C:\Python34` folder structure.

We also need the [MinGW](http://www.mingw.org/) package and also include some Unix commands including:

* grep
* wc

We also need the [Espressif SDK](http://bbs.espressif.com/viewtopic.php?f=46&t=850).

We also need the [esptool-ck](https://github.com/igrr/esptool-ck) utility.

##Source files
The source files for the ESP8266 project should terminate with Unix style line delimiters (LF).  When working on Eclipse on Windows, you need to specify this explicitly.  Open up `Preferences > General > Workspace` and change "New text file line delimiter" to "Unix".
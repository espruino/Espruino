#Emulation of Espruino on ESP8266 on Linux/Windows

This directory contains artifacts necessary for compiling an running Espruino from source such
that it thinks it is running on an ESP8266 but is actually running on a different platform such as
Linux or Windows.  The value of this is to be able to leverage value added development tools such
as debuggers and profilers against binaries where those tools may not be available for
embedded systems.

Since the execution of Espruino believes it is running on an ESP8266 then this also implies there
is a layer that is itself pretending to provide the services that are present on an ESP8266.

See Github issue: #610.
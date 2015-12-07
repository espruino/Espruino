Wifi tests
==========

This directory tree contains some simple tests for the Wifi library and the network sockets.
To run the Wifi library tests
- configure the IDE to use this directory for the "sandbox" (in settings >
project > directory for sandbox)
- configure the IDE to minify modules and "code from the editor window" (closure/whitespace is sufficient for the latter)
- configure the IDE to send a reset() for each upload
- open the `wifi-*.js` tests one after the other
- each time update the access point SSID and password at the top
- then run the program using the one-click "send to espruino" button
- some will pause 10-20 seconds at times while the Wifi is "doing something"

For the sockets tests you will need to follow pretty much the same procedure but in addition:
- run the ruby sinatra `http_test.rb` test server on a machine on your network, for this you will
  need ruby 2.x installed and you start it using `ruby http_test.rb -o 0.0.0.0`
- update the test_host at the top of the tests to point to the machine on which you are running
  the ruby server

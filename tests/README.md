Tests
=====

The tests here are run automatically when you type

```sh
./espruino --test-all
```

Ideally their name would represent what they tested, but this hasn't been done yet.

## Other tests

You can find an overview of all of these by running `./espruino --help`.

### Run a supplied test
```sh
./espruino --test test.js
```

### Run all Exhaustive Memory crash tests

```sh
./espruino --test-mem-all
```

### Run the supplied Exhaustive Memory crash test

```sh
./espruino --test-mem test.js
```

### Run the supplied Exhaustive Memory crash test with # vars

```sh
./espruino --test-mem-n test.js #
```


// https://github.com/espruino/Espruino/issues/2451


result = "blah".constructor == String.prototype.constructor;
result &= {}.constructor == Object.prototype.constructor;
trace();

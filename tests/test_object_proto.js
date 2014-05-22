// https://github.com/espruino/Espruino/issues/381

result = 
  Array.prototype === [].__proto__ &&
  String.prototype === "".__proto__ &&
  Number.prototype === (5).__proto__;

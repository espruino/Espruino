// https://github.com/espruino/Espruino/issues/290
var arr = ["a"];
result = ((arr[true] === undefined) === true) &&
          ((arr[false] === undefined) === true);

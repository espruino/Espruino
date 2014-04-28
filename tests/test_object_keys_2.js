// problem with deleting properties that doesn't exist #344
// https://github.com/espruino/Espruino/issues/344


var foo = {};
result = (delete foo.bar) == false;

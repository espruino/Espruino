// https://github.com/espruino/Espruino/issues/1875
let base = {foo: 42};
let child = {};
Object.setPrototypeOf(child, base);
print(child.__proto__ == Object.prototype);
print(Object.prototype);

result = (child.__proto__ != Object.prototype) && JSON.stringify(Object.prototype)=="{}";

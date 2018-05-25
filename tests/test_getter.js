// Examples from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/get

var results = [];

var obj = {
  log: ['a', 'b', 'c'],
  get latest() {
    if (this.log.length == 0) {
      return undefined;
    }
    return this.log[this.log.length - 1];
  }
}

results.push(obj.latest);
// expected output: "c"

var obj = {
  log: ['example','test'],
  get latest() {
    if (this.log.length == 0) return undefined;
    return this.log[this.log.length - 1];
  }
}
results.push(obj.latest); // "test".

delete obj.latest;
results.push(obj.latest); // undefined

var o = {a: 0};

Object.defineProperty(o, 'b', { get: function() { return this.a + 1; } });

results.push(o.b) // Runs the getter, which yields a + 1 (which is 1)

class Example {
  get hello() {
    return this.foobar;
  }
}
const obj = new Example();
obj.foobar = "world";
results.push(obj.hello); // "world"

result = results=="c,test,,1,world";

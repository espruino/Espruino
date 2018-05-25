// Examples from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/set

var results = [];

var language = {
  set current(name) {
    this.log.push(name);
  },
  log: []
}

language.current = 'EN';
language.current = 'FA';

results.push(JSON.stringify(language.log));
// expected output: Array ["EN", "FA"]

var language = {
  set current(name) {
    this.log.push(name);
  },
  log: []
}

language.current = 'EN';
results.push(JSON.stringify(language.log)); // ['EN']

language.current = 'FA';
results.push(JSON.stringify(language.log)); // ['EN', 'FA']

delete language.current;
language.current = 'EN';
results.push(JSON.stringify(language.log)); // ['EN', 'FA']

var o = {a: 0};

Object.defineProperty(o, 'b', { set: function(x) { this.a = x / 2; } });

o.b = 10; // Runs the setter, which assigns 10 / 2 (5) to the 'a' property
results.push(o.a) // 5


class Example {
  set hello(x) {
    console.log("Setting"+x);
    this.inner = x;
  }
}
const obj = new Example();
trace(obj);
obj.hello = "world"
results.push(obj.inner); // "world"

print(results+"");
result = results=='["EN","FA"],["EN"],["EN","FA"],["EN","FA"],5,world';

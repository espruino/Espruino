function A(a, b) {
  this.a = a;
  this.b = b;
  return this;
}

function B(a, b) {
  A.call(this, a, b);
  this.c = 42;
}
B.prototype = new A();

var x = new B('hello', 'world');

result = x.a == 'hello' && x.b == 'world' && x.c==42;


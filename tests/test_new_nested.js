// test for nested constructors

function A(x) {
 this.a = x;
}

function B() {
 this.b = 42;
}

var r = new A(new B());
result = r.a.b == 42;


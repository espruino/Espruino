// test for nested constructors

function A(x) {
 this.a = x;
}

function B() {
 this.b = 42;
}

result = new A(new B()) == 42;


// test for nested constructors

function A(x) {
 this.a = function() { this.a = 42; };
}

result = new new A().a().a == 42;


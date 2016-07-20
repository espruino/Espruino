// from MDN https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/instanceof

result = 1;
function test(s, r) {
  if (eval(s)!=r) {
    console.log(s+" should be "+r);
    result = 0; 
  }
}

// defining constructors
function C(){}
function D(){}

var o = new C();

// true, because: Object.getPrototypeOf(o) === C.prototype
test("o instanceof C", true);

// false, because D.prototype is nowhere in o's prototype chain
test("o instanceof D" ,false);

test("o instanceof Object", true); // true, because:
test("C.prototype instanceof Object", true); // true

C.prototype = {};
var o2 = new C();

test("o2 instanceof C", true); // true

// false, because C.prototype is nowhere in
// o's prototype chain anymore
test("o instanceof C", false); 

D.prototype = new C(); // use inheritance
var o3 = new D();
test("o3 instanceof D", true); // true
test("o3 instanceof C", true); // true


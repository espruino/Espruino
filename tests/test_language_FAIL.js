// From https://gist.github.com/randunel/8983226
// It's very unlikely this will ever pass completely, but it's good to have
// IMO it should be split into separate tests

var tests = 0;
var testsPass = 0;

// From V8 and SpiderMonkey
function shouldBe(item, value) {
  tests++;
  if (eval(item) !== eval(value)) {
    console.log('ERROR:', item, 'should be', value, 'but found', '' + eval(item));    
  } else testsPass++;
}

function shouldBeUndefined(item) {
  tests++;
  if (eval(item) !== undefined) {
    console.log('ERROR:', item, 'should be undefined but found', '' + eval(item));
  } else testsPass++;
}

function shouldBeTrue(item) {
  tests++;
  if (eval(item) !== true) {
    console.log('ERROR:', item, 'should be true but found', '' + eval(item));
  } else testsPass++;
}

function shouldBeFalse(item) {
  tests++;
  if (eval(item) !== false) {
    console.log('ERROR:', item, 'should be false but found', '' + eval(item));
  } else testsPass++;
}

function shouldBeOfType(msg, val, type) {
  tests++;
  if (typeof(val) !== type)
    console.log('ERROR:', msg + ": value has type " + typeof(val) + " , not:" + type);
  else testsPass++;
}

function shouldBeVal(msg, val, expected) {
  tests++;
  if (val !== expected)
    console.log('ERROR:', msg + ": value is " + val + " , not:" + expected);
  else testsPass++;
}

shouldBe("Array().length", "0");
shouldBe("(new Array()).length", "0");
shouldBe("(new Array(3)).length", "3");
shouldBe("(new Array(11, 22)).length", "2");
shouldBe("(new Array(11, 22))[0]", "11");
shouldBe("Array(11, 22)[1]", "22");
shouldBeUndefined("(new Array(11, 22))[3]");
shouldBe("String(new Array(11, 22))", "'11,22'");
shouldBe("var a = []; a[0] = 33; a[0]", "33");
shouldBe("var a = []; a[0] = 33; a.length", "1");

shouldBe("Array().toString()", "''");
shouldBe("Array(3).toString()", "',,'");
shouldBe("Array(11, 22).toString()", "'11,22'");

shouldBe("[1,2,3,4].slice(1, 3).toString()", "'2,3'");
shouldBe("[1,2,3,4].slice(-3, -1).toString()", "'2,3'");
shouldBe("[1,2].slice(-9, 0).length", "0");
shouldBe("[1,2].slice(1).toString()", "'2'");
shouldBe("[1,2].slice().toString()", "'1,2'");

shouldBe("(new Array('a')).length", "1");
shouldBe("(new Array('a'))[0]", "'a'");
shouldBeUndefined("(new Array('a'))[1]");

shouldBe("Array('a').length", "1");
shouldBe("Array('a')[0]", "'a'");

shouldBe("String(Array())", "''");
shouldBe("String(Array('a','b'))", "'a,b'");

shouldBe("[].length", "0");
shouldBe("['a'].length", "1");
shouldBe("['a'][0]", "'a'");
shouldBe("['a',undefined,'c'][1]", "undefined");
shouldBe("1 in ['a',undefined,'c']", "true");

var arrayWithDeletion = ['a','b','c'];
delete arrayWithDeletion[1];
shouldBe("1 in arrayWithDeletion", "false");

function forInSum(_a) {
  var s = '';
  for (var i in _a)
    s += _a[i];
  return s;
}

shouldBe("forInSum([])", "''");
shouldBe("forInSum(Array())", "''");
shouldBe("forInSum(Array('a'))", "'a'");

var a0 = [];
shouldBe("forInSum(a0)", "''");

var a1 = [ 'a' ];
shouldBe("forInSum(a1)", "'a'");

shouldBe("String(['a', 'b', 'c'].splice(1, 2, 'x', 'y'))", "'b,c'");

var arr = new Array('a','b','c');
var propnames = [];
for (var i in arr)
  propnames.push(i);
shouldBe("propnames.length","3");
shouldBe("propnames[0]","'0'");
shouldBe("propnames[1]","'1'");
shouldBe("propnames[2]","'2'");



var h = "a\xefc";
var u = "a\u1234c";
var z = "\x00";

shouldBe("h.charCodeAt(1)", "239");
shouldBe("u.charCodeAt(1)", "4660");

shouldBeTrue("isNaN(NaN)");
shouldBeTrue("isNaN('NaN')");
shouldBeFalse("isNaN('1')");

// all should return NaN because 1st char is non-number
shouldBe('isNaN(parseInt("Hello", 8))', "true");
shouldBe('isNaN(parseInt("FFF", 10))', "true");
shouldBe('isNaN(parseInt(".5", 10))', "true");

shouldBeTrue("isNaN(parseInt())");
shouldBeTrue("isNaN(parseInt(''))");
shouldBeTrue("isNaN(parseInt(' '))");
shouldBeTrue("isNaN(parseInt('a'))");
shouldBe("parseInt(1)", "1");
shouldBe("parseInt(1234567890123456)", "1234567890123456");
shouldBe("parseInt(1.2)", "1");
shouldBe("parseInt(' 2.3')", "2");
shouldBe("parseInt('0x10')", "16");
shouldBe("parseInt('11', 0)", "11");
shouldBe("parseInt('F', 16)", "15");

shouldBeTrue("isNaN(parseInt('10', 40))");
shouldBe("parseInt('3x')", "3");
shouldBe("parseInt('3 x')", "3");
shouldBeTrue("isNaN(parseInt('Infinity'))");

// all should return 15
shouldBe('parseInt("15")', "15");
shouldBe('parseInt("015")', "15"); // ES5 prohibits parseInt from handling octal, see annex E.
shouldBe('parseInt("0xf")', "15");
shouldBe('parseInt("15", 0)', "15");
shouldBe('parseInt("15", 10)', "15");
shouldBe('parseInt("F", 16)', "15");
shouldBe('parseInt("17", 8)', "15");
shouldBe('parseInt("15.99", 10)', "15");
shouldBe('parseInt("FXX123", 16)', "15");
shouldBe('parseInt("1111", 2)', "15");
shouldBe('parseInt("15*3", 10)', "15");

// this should be 0
shouldBe('parseInt("0x7", 10)', "0");
shouldBe('parseInt("1x7", 10)', "1");

shouldBeTrue("isNaN(parseFloat())");
shouldBeTrue("isNaN(parseFloat(''))");
shouldBeTrue("isNaN(parseFloat(' '))");
shouldBeTrue("isNaN(parseFloat('a'))");
shouldBe("parseFloat(1)", "1");
shouldBe("parseFloat(' 2.3')", "2.3");
shouldBe("parseFloat('3.1 x', 3)", "3.1");
shouldBe("parseFloat('3.1x', 3)", "3.1");
shouldBeFalse("delete NaN");
shouldBeFalse("delete Infinity");
shouldBeFalse("delete undefined");


shouldBe("isNaN(Number('a'))", "true");
shouldBe("isNaN(new Number('a'))", "true");

shouldBe("isNaN(Number.NaN)", "true");
shouldBe("Number.NEGATIVE_INFINITY", "-Infinity");
shouldBe("Number.POSITIVE_INFINITY", "Infinity");

shouldBe("(1).toString()", "'1'");
shouldBe("typeof (1).toString()", "'string'");
shouldBe("(10).toString(16)", "'a'");
shouldBe("(8.5).toString(16)", "'8.8'");
shouldBe("(-8.5).toString(16)", "'-8.8'");
shouldBe("Number.POSITIVE_INFINITY.toString(16)", "'Infinity'");
shouldBe("Number.NEGATIVE_INFINITY.toString(16)", "'-Infinity'");
shouldBe("Number.MAX_VALUE.toString(2).length", "23");
shouldBe("typeof 1", "'number'");




shouldBe("typeof (new Object())", "'object'");
shouldBe("var o = new Object(); o.x = 11; o.x;", "11");

shouldBe("typeof (new Object())", "'object'");

shouldBe("String(new Object())", "'[object Object]'");
shouldBe("(new Object()).toString()", "'[object Object]'");




function Square(x) {
  this.x = x;
}

new Square(0); // create prototype

function Square_area() { return this.x * this.x; }
Square.prototype.area = Square_area;
var s = new Square(3);
shouldBe("s.area()", "9");

function Item(name) {
  this.name = name;
}

function Book(name, author){
  this.base = Item;         // set Item constructor as method of Book object
  this.base(name);           // set the value of name property
  this.author = author;
}
Book.prototype = new Item();
var b = new Book("a book", "Fred");        // create object instance
//edebug(e"b.name"));
shouldBe("b.name", "'a book'");
shouldBe("b.author", "'Fred'");                  // outpus "Fred"

shouldBe("delete Array.prototype", "false");

shouldBe("var i = 1; i", "1");
shouldBe("j = k = 2", "2");
shouldBeUndefined("var i; i");

// compound assignments
shouldBe("var i = 1; i <<= 2", "4");
shouldBe("var i = 8; i >>= 1", "4");
shouldBe("var i = 1; i >>= 2", "0");
shouldBe("var i = -8; i >>= 24", "-1");
shouldBe("var i = 8; i >>>= 2", "2");
shouldBe("var i = -8; i >>>= 24", "255");

var i = 1;

function foo() {
  i = 2;
  return;
  i = 3;
}

shouldBe("foo(), i", "2");

// value completions take precedence
var val = eval("11; { }");
shouldBe("val", "11");
val = eval("12; ;");
shouldBe("val", "12");
val = eval("13; if(false);");
shouldBe("val", "13");
val = eval("14; function f() {}");
shouldBe("val", "14");
val = eval("15; var v = 0");
shouldBe("val", "15");

shouldBe("true ? 1 : 2", "1");
shouldBe("false ? 1 : 2", "2");
shouldBe("'abc' ? 1 : 2", "1");
shouldBe("null ? 1 : 2", "2");
shouldBe("undefined ? 1 : 2", "2");
var asd = 1;
if ( undefined )
  asd = 2;
shouldBe("/*var asd=1;if (undefined) asd = 2;*/ asd", "1");


shouldBe("a = 1; delete a;", "true");
shouldBe("delete nonexistant;", "true");
shouldBe("delete NaN", "false");


f = "global";

function test() {
  shouldBeOfType("Function declaration takes effect at entry", f, "function");

  for (var i = 0; i < 3; ++i) {
    if (i == 0)
      shouldBeOfType("Decl not yet overwritten", f, 'function');
    else
      shouldBeOfType("Decl already overwritten", f, 'number');

    f = 3;
    shouldBeVal("After assign ("+i+")", f, 3);

    function f() {};
    shouldBeVal("function decls have no execution content", f, 3);

    f = 5;

    shouldBeVal("After assign #2 ("+i+")", f, 5);
  }
}

test();


var count = 0;
do {
  count++;
} while (count < 10);
shouldBe("count", "10");

count = 0;
for (var i = 0; i < 10; i++) {
  if (i == 5)
    break;
  count++;
}
shouldBe("count", "5");

count = 0;
for (i = 0; i < 10; i++) {
  count++;
}
shouldBe("count", "10");

obj = new Object();
obj.a = 11;
obj.b = 22;

properties = "";
for ( prop in obj ) {
  properties += (prop + "=" + obj[prop] + ";");
}

shouldBe("properties", "'a=11;b=22;'");

obj.y = 33;
obj.x = 44;
properties = "";
for ( prop in obj )
  properties += prop;

arr = new Array;
arr[0] = 100;
arr[1] = 101;
list = "";
for ( var j in arr ) {
  list += "[" + j + "]=" + arr[j] + ";";
}
shouldBe("list","'[0]=100;[1]=101;'");

list = "";
for (var a = [1,2,3], length = a.length, i = 0; i < length; i++) {
  list += a[i];
}
shouldBe("list", "'123'");


var negativeZero = Math.atan2(-1, Infinity); // ### any nicer way?

function isNegativeZero(n) {
  return n == 0 && 1 / n < 0;
}

// self tests
shouldBeTrue("isNegativeZero(negativeZero)");
shouldBeFalse("isNegativeZero(0)");

// Constants
shouldBe("String()+Math.E", "'2.718281828459045'");
shouldBe("String()+Math.LN2", "'0.6931471805599453'");
shouldBe("String()+Math.LN10", "'2.302585092994046'");
shouldBe("String()+Math.LOG2E", "'1.4426950408889634'");
shouldBe("String()+Math.LOG10E", "'0.4342944819032518'");
shouldBe("String()+Math.PI", "'3.141592653589793'");
shouldBe("String()+Math.SQRT1_2", "'0.7071067811865476'");
shouldBe("String()+Math.SQRT2", "'1.4142135623730951'");

shouldBe("String()+Number.NaN", "'NaN'");
shouldBe("String()+Number.NEGATIVE_INFINITY", "'-Infinity'");
shouldBe("String()+Number.POSITIVE_INFINITY", "'Infinity'");


shouldBe("Math.abs(-5)", "5");
shouldBe("Math.acos(0)", "Math.PI/2");
shouldBe("Math.acos(1)", "0");
shouldBe("Math.ceil(1.1)", "2");
shouldBe("String()+Math.sqrt(2)", "String()+Math.SQRT2");
shouldBe("Math.ceil(1.6)", "2");
shouldBe("Math.round(0)", "0");
shouldBeFalse("isNegativeZero(Math.round(0))");
shouldBeTrue("isNegativeZero(Math.round(negativeZero))");
shouldBe("Math.round(0.2)", "0");
shouldBeTrue("isNegativeZero(Math.round(-0.2))");
shouldBeTrue("isNegativeZero(Math.round(-0.5))");
shouldBe("Math.round(1.1)", "1");
shouldBe("Math.round(1.6)", "2");
shouldBe("Math.round(-3.5)", "-3");
shouldBe("Math.round(-3.6)", "-4");
shouldBeTrue("isNaN(Math.round())");
shouldBeTrue("isNaN(Math.round(NaN))");
shouldBe("Math.round(-Infinity)", "-Infinity");
shouldBe("Math.round(Infinity)", "Infinity");
shouldBe("Math.round(99999999999999999999.99)", "100000000000000000000");
shouldBe("Math.round(-99999999999999999999.99)", "-100000000000000000000");

shouldBe("Math.log(Math.E*Math.E)", "2");
shouldBeTrue("isNaN(Math.log(NaN))");
shouldBeTrue("isNaN(Math.log(-1))");
shouldBe("Math.log(1)", "0");


var obj = {};
obj.a = 1;
obj.b = 2;
list="";
for ( var i in obj ) { list += i + ','; }
shouldBe("list","'a,b,'");

list="";
for ( var i in Math ) { list += i + ','; }
shouldBe("list","''");

Math.myprop=true; // adding a custom property to the math object (why not?)
list="";
for ( var i in Math ) { list += i + ','; }
shouldBe("list","'myprop,'");


shouldBeTrue("!undefined");
shouldBeTrue("!null");
shouldBeTrue("!!true");
shouldBeTrue("!false");
shouldBeTrue("!!1");
shouldBeTrue("!0");
shouldBeTrue("!!'a'");
shouldBeTrue("!''");
// unary plus
shouldBe("+9", "9");
shouldBe("var i = 10; +i", "10");

// negation
shouldBe("-11", "-11");
shouldBe("var i = 12; -i", "-12");

// increment
shouldBe("var i = 0; ++i;", "1");
shouldBe("var i = 0; ++i; i", "1");
shouldBe("var i = 0; i++;", "0");
shouldBe("var i = 0; i++; i", "1");
shouldBe("var i = true; i++", "1");
shouldBe("var i = true; i++; i", "2");

// decrement
shouldBe("var i = 0; --i;", "-1");
shouldBe("var i = 0; --i; i", "-1");
shouldBe("var i = 0; i--;", "0");
shouldBe("var i = 0; i--; i", "-1");
shouldBe("var i = true; i--", "1");
shouldBe("var i = true; i--; i", "0");


var one = 1;
var two = 2;
var twentyFour = 24;

shouldBe("1 << two", "4");
shouldBe("8 >> one", "4");
shouldBe("1 >> two", "0");
shouldBe("-8 >> twentyFour", "-1");
shouldBe("8 >>> two", "2");
shouldBe("-8 >>> twentyFour", "255");
shouldBe("(-2200000000 >> one) << one", "2094967296");
shouldBe("Infinity >> one", "0");
shouldBe("Infinity << one", "0");
shouldBe("Infinity >>> one", "0");
shouldBe("NaN >> one", "0");
shouldBe("NaN << one", "0");
shouldBe("NaN >>> one", "0");
shouldBe("888.1 >> one", "444");
shouldBe("888.1 << one", "1776");
shouldBe("888.1 >>> one", "444");
shouldBe("888.9 >> one", "444");
shouldBe("888.9 << one", "1776");
shouldBe("888.9 >>> one", "444");
shouldBe("Math.pow(2, 32) >> one", "0");
shouldBe("Math.pow(2, 32) << one", "0");
shouldBe("Math.pow(2, 32) >>> one", "0");

// addition
shouldBe("1+2", "3");
shouldBe("'a'+'b'", "'ab'");
shouldBe("'a'+2", "'a2'");
shouldBe("'2'+'-1'", "'2-1'");
shouldBe("true+'a'", "'truea'");
shouldBe("'a' + null", "'anull'");
shouldBe("true+1", "2");
shouldBe("false+null", "0");

// substraction
shouldBe("1-3", "-2");
shouldBe("isNaN('a'-3)", "true");
shouldBe("'3'-'-1'", "4");
shouldBe("'4'-2", "2");
shouldBe("true-false", "1");
shouldBe("false-1", "-1");
shouldBe("null-true", "-1");

// multiplication
shouldBe("2 * 3", "6");
shouldBe("true * 3", "3");
shouldBe("2 * '3'", "6");

// division
shouldBe("6 / 4", "1.5");
//shouldBe("true / false", "Inf");
shouldBe("'6' / '2'", "3");
shouldBeTrue("isNaN('x' / 1)");
shouldBeTrue("isNaN(1 / NaN)");
shouldBeTrue("isNaN(Infinity / Infinity)");
shouldBe("Infinity / 0", "Infinity");
shouldBe("-Infinity / 0", "-Infinity");
shouldBe("Infinity / 1", "Infinity");
shouldBe("-Infinity / 1", "-Infinity");
shouldBeTrue("1 / Infinity == +0");
shouldBeTrue("1 / -Infinity == -0"); // how to check ?
shouldBeTrue("isNaN(0/0)");
shouldBeTrue("0 / 1 === 0");
shouldBeTrue("0 / -1 === -0"); // how to check ?
shouldBe("1 / 0", "Infinity");
shouldBe("-1 / 0", "-Infinity");

// modulo
shouldBe("6 % 4", "2");
shouldBe("'-6' % 4", "-2");

shouldBe("2==2", "true");
shouldBe("1==2", "false");




shouldBe("1<2", "true");
shouldBe("1<=2", "true");
shouldBe("2<1", "false");
shouldBe("2<=1", "false");

shouldBe("2>1", "true");
shouldBe("2>=1", "true");
shouldBe("1>=2", "false");
shouldBe("1>2", "false");


shouldBeTrue("'abc' == 'abc'");
shouldBeTrue("'abc' != 'xyz'");
shouldBeTrue("true == true");
shouldBeTrue("false == false");
shouldBeTrue("true != false");
shouldBeTrue("'a' != null");
shouldBeTrue("'a' != undefined");
shouldBeTrue("null == null");
shouldBeTrue("null == undefined");
shouldBeTrue("undefined == undefined");
shouldBeTrue("NaN != NaN");
shouldBeTrue("true != undefined");
shouldBeTrue("true != null");
shouldBeTrue("false != undefined");
shouldBeTrue("false != null");
shouldBeTrue("'0' == 0");
shouldBeTrue("1 == '1'");
shouldBeTrue("NaN != NaN");
shouldBeTrue("NaN != 0");
shouldBeTrue("NaN != undefined");
shouldBeTrue("true == 1");
shouldBeTrue("true != 2");
shouldBeTrue("1 == true");
shouldBeTrue("false == 0");
shouldBeTrue("0 == false");


shouldBe("'abc' < 'abx'", "true");
shouldBe("'abc' < 'abcd'", "true");
shouldBe("'abc' < 'abc'", "false");
shouldBe("'abcd' < 'abcd'", "false");
shouldBe("'abx' < 'abc'", "false");


shouldBe("'abc' <= 'abc'", "true");
shouldBe("'abc' <= 'abx'", "true");
shouldBe("'abx' <= 'abc'", "false");
shouldBe("'abcd' <= 'abc'", "false");
shouldBe("'abc' <= 'abcd'", "true");


shouldBe("'abc' > 'abx'", "false");
shouldBe("'abc' > 'abc'", "false");
shouldBe("'abcd' > 'abc'", "true");
shouldBe("'abx' > 'abc'", "true");
shouldBe("'abc' > 'abcd'", "false");


shouldBe("'abc' >= 'abc'", "true");
shouldBe("'abcd' >= 'abc'", "true");
shouldBe("'abx' >= 'abc'", "true");
shouldBe("'abc' >= 'abx'", "false");
shouldBe("'abc' >= 'abx'", "false");
shouldBe("'abc' >= 'abcd'", "false");


shouldBeFalse("'abc' <= 0"); // #35246
shouldBeTrue("'' <= 0");
shouldBeTrue("' ' <= 0");
shouldBeTrue("null <= 0");
shouldBeFalse("0 <= 'abc'");
shouldBeTrue("0 <= ''");
shouldBeTrue("0 <= null");
shouldBeTrue("null <= null");
shouldBeTrue("6 < '52'");
shouldBeTrue("6 < '72'"); // #36087
shouldBeFalse("NaN < 0");
shouldBeFalse("NaN <= 0");
shouldBeFalse("NaN > 0");
shouldBeFalse("NaN >= 0");


// strict comparison ===
shouldBeFalse("0 === false");
shouldBeTrue("null === null");
shouldBeFalse("NaN === NaN");
shouldBeTrue("0.0 === 0");
shouldBeTrue("'abc' === 'abc'");
shouldBeFalse("'a' === 'x'");
shouldBeFalse("1 === '1'");
shouldBeFalse("'1' === 1");
shouldBeTrue("true === true");
shouldBeTrue("false === false");
shouldBeFalse("true === false");
shouldBeTrue("Math === Math");
shouldBeFalse("Math === Boolean");
shouldBeTrue("Infinity === Infinity");


shouldBe("0 !== 0", "false");
shouldBe("0 !== 1", "true");


shouldBe("typeof undefined", "'undefined'");
shouldBe("typeof null", "'object'");
shouldBe("typeof true", "'boolean'");
shouldBe("typeof false", "'boolean'");
shouldBe("typeof 1", "'number'");
shouldBe("typeof 'a'", "'string'");
shouldBe("typeof shouldBe", "'function'");
shouldBe("typeof Number.NaN", "'number'");


shouldBe("11 && 22", "22");
shouldBe("null && true", "null");
shouldBe("11 || 22", "11");
shouldBe("null || 'a'", "'a'");

shouldBeUndefined("void 1");

shouldBeTrue("1 in [1, 2]");
shouldBeFalse("3 in [1, 2]");
shouldBeTrue("'a' in { a:1, b:2 }");

// Make sure that for ... in reevaluates the scoping every time!
var P = { foo : 1, bar : 2, baz : 3 };

function testForIn() {
   for (g in P) {
        eval("var g;"); //Change the scope of g half-ways through the loop
   }
}

testForIn();
shouldBe("g", "'foo'"); //Before the eval, g was in outer scope, but not after!


function testSwitch(v) {
  var result = "";
  switch (v) {
     case 0: result += 'a';
     case 1: result += 'b';
     case 1: result += 'c';
     case 2: result += 'd'; break;
  }
  return result;
}

shouldBe("testSwitch(0)", "'abcd'");
shouldBe("testSwitch(1)", "'bcd'"); // IE agrees, NS disagrees
shouldBe("testSwitch(2)", "'d'");
shouldBe("testSwitch(false)", "''");

function testSwitch4(v) {
  var result = "";
  switch (v) {
     case 0: result += 'a'; result += 'b'; break;
  }
  return result;
};

shouldBe("testSwitch4(0)", "'ab'");

var myvar = 1;

function varInFunction() {
  return (myvar == undefined);
  var myvar = 2;
}

function varInVarList() {
  return (myvar == undefined);
  var a = 1, b = 0, myvar = 2;
}

function varListOrder() {
  var tmp = 0;
  var i = ++tmp, j = ++tmp;
  return j == 2;
}

function varInBlock() {
  return (myvar == undefined);
  {
    var myvar = 2;
  }
}

function varInIf() {
  return (myvar == undefined);
  if (false)
    var myvar = 2;
}

function varInElse() {
  return (myvar == undefined);
  if (true) {
  }
  else
    var myvar = 2;
}

function varInDoWhile() {
  return (myvar == undefined);
  do
    var myvar = 2;
  while (false);
}

function varInWhile() {
  return (myvar == undefined);
  while (false)
    var myvar = 2;
}

function varInFor() {
  return (myvar == undefined);
  var i;
  for (i = 0; i < 0; i++)
    var myvar = 2;
}

function varInForInitExpr() {
  return (myvar == undefined);
  for (var myvar = 2; i < 2; i++) {
  }
}

function varInWith() {
  return (myvar == undefined);
  with (String)
    var myvar = 2;
}

function varInCase() {
  return (myvar == undefined);
  switch (1) {
    case 0: var myvar = 2;
    case 1:
  }
}

if (!varGlobal)
  var varGlobal = 1;

shouldBe("varInFunction()","true");
shouldBe("varInVarList()","true");
shouldBe("varListOrder()","true");
shouldBe("varInBlock()","true");
shouldBe("varInIf()","true");
shouldBe("varInElse()","true");
shouldBe("varInDoWhile()","true");
shouldBe("varInWhile()","true");
shouldBe("varInFor()","true");
shouldBe("varInWith()","true");
shouldBe("varInCase()","true");
shouldBe("varInForInitExpr()","true");
shouldBe("varGlobal", "1");

var overrideVar = 1;
var overrideVar;
shouldBe("overrideVar", "1");

var overrideVar2 = 1;
var overrideVar2 = 2;
shouldBe("overrideVar2", "2");

console.log(testsPass+" out of "+tests+" passed");
result = testsPass == tests;


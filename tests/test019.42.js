// built-in functions

foo = "foo bar stuff";
// 42-tiny-js change begin --->
// in JavaScript this function is called Math.random()
//r = Math.rand();
r = Math.random();
//<--- 42-tiny-js change end

// 42-tiny-js change begin --->
// in JavaScript parseInt is a methode in the global scope (root-scope)
//parsed = Integer.parseInt("42");
parsed = parseInt("42");
//<--- 42-tiny-js change end

aStr = "ABCD";
aChar = aStr.charAt(0);

obj1 = new Object();
obj1.food = "cake";
obj1.desert = "pie";

obj2 = obj1.clone();
obj2.food = "kittens";

result = foo.length==13 && foo.indexOf("bar")==4 && foo.substring(8,13)=="stuff" && parsed==42 &&
// 42-tiny-js change begin --->
// in 42tiny-js the Integer-Objecte will be removed
// Integer.valueOf can be replaced by String.charCodeAt
//       Integer.valueOf(aChar)==65 && obj1.food=="cake" && obj2.desert=="pie";
         aChar.charCodeAt()==65 && obj1.food=="cake" && obj2.desert=="pie";
//<--- 42-tiny-js change end

var tests=0, testpass=0;
function t(x) {
 console.log(x);
 tests++;
 if (x) testpass++;
}


t( typeof undefined === "undefined" );
t( typeof null === "object" );
t( typeof true === "boolean" );
t( typeof false === "boolean" );
t( typeof "foo" === "string" );
t( typeof 1 === "number" );
t( typeof function() {} === "function" );

result = tests==testpass;

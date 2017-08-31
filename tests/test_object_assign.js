var tests=0,testpass=0;

function test(a,b) {
  tests++;
  if (JSON.stringify(a)==JSON.stringify(b))
    testpass++;
  else
    console.log("Got",a,"expected", b);
}

test(Object.assign({}, {a:6}),{a:6});
test(Object.assign({}, { a: 1, b: 1, c: 1 }, { b: 2, c: 2 }, { c: 3 }), { a: 1, b: 2, c: 3 });

result = tests==testpass;

var tests=0,testpass=0;

function test(a,b) {
  tests++;
  if (JSON.stringify(a)==JSON.stringify(b))
    testpass++;
  else
    console.log("Got",a,"expected", b);
}

test(Object.fromEntries([['a',1],['b','c'],['d',true]]),{ a:1, b:'c', d:true });
test(Object.fromEntries([]),{});

result = tests==testpass;

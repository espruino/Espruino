var A = {
  b : 42,
  c : function() {}
};

class B {
};


var r = [
 // [JSON.stringify(Object.getOwnPropertyDescriptors(A)),'{"b":{"writable":true,"enumerable":true,"configurable":true,"value":42},"c":{"writable":true,"enumerable":true,"configurable":true}}'],
  // this one is not correct as it seems V8 adds 'length' and 'name' fields to classes
  //[JSON.stringify(Object.getOwnPropertyDescriptors(B)),"{\"prototype\":{\"writable\":true,\"enumerable\":true,\"configurable\":true,\"value\":{}}}"]
];

//console.log(r);

result = r.every(a => a[0]==a[1]);


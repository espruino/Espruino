function test(a,b) {
  var ea = eval(a);  
  if (JSON.stringify(ea)!=JSON.stringify(b)) {
    console.log(JSON.stringify(a)+" should be "+JSON.stringify(b)+", got "+JSON.stringify(ea))
    result = 0;
  }
}


function concat_a() {
  return this+":"+arguments.join(",");
}
function concat_b(a) {
  return this+":"+arguments.join(",");
}
function concat_c(a,b) {  
  return this+":"+arguments.join(",");
}

result = 1;
test("concat_a.bind('Hello','There')('World')","Hello:There,World");
test("concat_a.bind('Hello','There').call(undefined,'World')","Hello:There,World");
test("concat_b.bind('Hello','There')('World')","Hello:There,World");
test("concat_c.bind('Hello','There')('World')","Hello:There,World");
test("concat_c.bind('Hello','There','Teeny','Tiny')('World')","Hello:There,Teeny,Tiny,World");

// bind x2
test("concat_c.bind('Hello','There').bind('Hello','Teeny','Tiny')('World')","Hello:There,Teeny,Tiny,World");

test("Math.max.bind(undefined,42)(23)",42);
test("Math.max.bind(undefined,42)(51)",51);
test("Math.max.bind(undefined,42).call(undefined,23)",42);
test("Math.max.bind(undefined,42).call(undefined,51)",51);


function test(a,b) {
  var ea = eval(a);  
  if (JSON.stringify(ea)!=JSON.stringify(b)) {
    console.log(JSON.stringify(a)+" should be "+JSON.stringify(b)+", got "+JSON.stringify(ea))
    result = 0;
  }
}

result = 1;
test("'abc'.padStart(10);         ","       abc");
test("'abc'.padStart(10, 'foo');  ","foofoofabc");
test("'abc'.padStart(6,'123465'); ","123abc");
test("'abc'.padStart(8, '0');     ","00000abc");
test("'abc'.padStart(1);          ","abc");
test("'abc'.padEnd(10);          ", "abc       ");
test("'abc'.padEnd(10, 'foo');   ", "abcfoofoof");
test("'abc'.padEnd(6, '123456'); ", "abc123");
test("'abc'.padEnd(1);           ", "abc");

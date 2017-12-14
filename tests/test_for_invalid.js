// test262 test/language/statements/for/S12.6.3_A8_T2.js causes assert fail

var arr = [];

function c() {
  for(index=0; {index++;index<100;}; index*2;);
}

try {
  c();
} catch (e) {
  result = 1;
}


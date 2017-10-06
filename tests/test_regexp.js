tests=0;
testPass=0;

function test(regex, matches, index) {
  tests++;
  if (regex==matches) {
    if (matches==null || regex.index==index)
      return testPass++;
  }
  console.log("Test "+tests+" failed, got ",regex);
}

test(new RegExp("a").exec("bc"), null);
test(/a/.exec("abcdef"),"a",0);
test(/a/.exec("bcdaef"),"a",3);
test(/a(b)c/.exec("abc"),"abc,b",0);
test(/\sWorld/.exec("Hello World")," World",5);
test(/a*b/.exec("Helloaaabc"),"aaab", 5);
test(/[bac]*d/.exec("Hello abcd"),"abcd", 6);

result = tests==testPass;
console.log("Pass",result);

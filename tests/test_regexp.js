tests=0;
testPass=0;

function testreg(regex, matches, index) {
  tests++;
  if (regex==matches) {
    if (matches==null || regex.index==index)
      return testPass++;
  }
  console.log("Test "+tests+" failed, got ",regex);
}
function test(a, b) {
  tests++;
  if (a==b) {
    return testPass++;
  }
  console.log("Test "+tests+" failed, got ",regex);
}

testreg(new RegExp("a").exec("bc"), null);
testreg(/a/.exec("abcdef"),"a",0);
testreg(/a/.exec("bcdaef"),"a",3);
testreg(/a(b)c/.exec("abc"),"abc,b",0);
testreg(/\sWorld/.exec("Hello World")," World",5);
testreg(/a*b/.exec("Helloaaabc"),"aaab", 5);
testreg(/[bac]*d/.exec("Hello abcd"),"abcd", 6);

test("Hellowa worldssaaa a a a".replace(/a/,""),"Hellow worldssaaa a a a");
test("Hellowa worldssaaa a a a".replace(/a/g,""),"Hellow worldss   ");
test("Hello".replace(/(l)/g,"[$1]"), "He[l][l]o");
test("Hello".replace(/e(l)/g,"[$1]"), "H[l]lo");

result = tests==testPass;
console.log("Pass",result);

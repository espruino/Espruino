let unicodeRemap = {
  0x20ac:"\x80", // Euro symbol
  0x2026:"\x85", // Ellipsis
};
print(unicodeRemap)

var testsFail = 0;

function expect(a, ex) {
  if (a!=ex) {
    print(`Test fail, got ${E.toJS(a)} expected ${E.toJS(ex)}`);
    testsFail++;
  }
}

//print("Extended ASCII Euro: \u0080");
expect(E.decodeUTF8("UTF-8 Euro: \xe2\x82\xac", unicodeRemap, '[?]'), "UTF-8 Euro: \x80"); // Works OK

//print("Extended ASCII ellipsis: \u0085");
expect(E.decodeUTF8("UTF-8 Ellipsis: \xe2\x80\xa6", unicodeRemap, '[?]'), "UTF-8 Ellipsis: \x85"); // Doesn't work

result = testsFail==0;

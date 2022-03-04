let unicodeRemap = {
  // Use lowercase unicode hex values!
  0x20ac:"\u0080", // Euro symbol
  0x2026:"\u0085", // Ellipsis
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
expect(E.decodeUTF8("UTF-8 Euro: \u00e2\u0082\u00ac", unicodeRemap, '[?]'), "UTF-8 Euro: \u0080"); // Works OK

//print("Extended ASCII ellipsis: \u0085");
expect(E.decodeUTF8("UTF-8 Ellipsis: \u00e2\u0080\u00a6", unicodeRemap, '[?]'), "UTF-8 Ellipsis: \u0085"); // Doesn't work

result = testsFail==0;

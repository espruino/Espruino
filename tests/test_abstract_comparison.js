// https://github.com/espruino/Espruino/issues/283

var abstract = 0;
var strict = 0;
var reference = {};

// "lref rref == ==="
var comparisons = [
  { l: undefined, r: undefined, a: true, s: true },
  { l: null, r: null, a: true, s: true },
  { l: true, r: true, a: true, s: true },
  { l: false, r: false, a: true, s: true },
  { l: 'foo', r: 'foo', a: true, s: true },
  { l: reference, r: reference, a: true, s: true },
  { l: 0, r: 0, a: true, s: true },
  { l: +0, r: -0, a: true, s: true },
  { l: 0, r: false, a: true, s: false },
  { l: '', r: false, a: true, s: false },
  { l: '', r: 0, a: true, s: false },
  { l: '0', r: 0, a: true, s: false },
  { l: '17', r: 17, a: true, s: false },
  // Fails because new String() returns the wrong type
  // { l: new String('foo'), r: 'foo', a: true, s: false },
  { l: null, r: undefined, a: true, s: false },
  { l: null, r: false, a: false, s: false },
  { l: undefined, r: false, a: false, s: false },
  { l: {}, r: {}, a: false, s: false },
  // Fails because new String() returns the wrong type
  // { l: new String('foo'), r: new String('foo'), a: false, s: false },
  { l: [1, 2], r: '1,2', a: true, s: false },
  { l: [0], r: '0', a: true, s: false },
  { l: 0, r: null, a: false, s: false },
  { l: 0, r: NaN, a: false, s: false },
  { l: 'foo', r: NaN, a: false, s: false },
  { l: NaN, r: NaN, a: false, s: false }
];
var c = null;

for (var i = 0; i < comparisons.length; i++) {
  c = comparisons[i];

  if ((c.l == c.r) === c.a) {
    abstract++;
  } else {
    console.log( "abstract test failed?", i, c );
  }

  if ((c.l === c.r) === c.s) {
    strict++;
  } else {
    console.log( "strict test failed?", i, c );
  }
}

result = ((comparisons.length === abstract) === true) &&
            ((comparisons.length === strict) === true);

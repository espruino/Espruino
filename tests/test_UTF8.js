require("Storage").writeJSON("utf8","Fön Kür Bär");
g = Graphics.createArrayBuffer(32,16,1);
g.drawString("F\u00F6n "+atob("AAoKATOc57/vc9zgAc5zjMA=")); // test image parsing in text is ok

// Get underlying numeric values with "B\xE4r F\xF6n" .split("").map(n=>"0x"+n.charCodeAt().toString(16).padStart(2,0)).join(", ")

var tests = [

["B\xE4r F\xF6n", E.toString([ 0x42, 0xe4, 0x72, 0x20, 0x46, 0xf6, 0x6e ])], // in Espruino using \x## doesn't force Unicode
["B\xE4r \u03C0", E.asUTF8(E.toString([ 0x42, 0xc3, 0xA4, 0x72, 0x20, 0xcf, 0x80 ]))], // the \u#### forces Unicode (including already-parsed chars)

[E.isUTF8("\u03C0"), true],
[E.isUTF8("\xE4"), false],

[E.fromUTF8("\u03C0"), "\xCF\x80"],

["Ωx".charCodeAt(0), 937],
["Ωx".charCodeAt(1), 120],
["xΩ".charCodeAt(1), 937],
["Ωx".charAt(0), "Ω"],
["Ωx".charAt(1), "x"],
["xΩy"[0], "x"],
["xΩy"[1], "Ω"],
["xΩy"[2], "y"],
["\u03A9", "Ω"], // NO unicode surrogates
[JSON.parse('"\u03A9"'), "Ω"], // NO unicode surrogates in JSON
[JSON.stringify('Ω'), '"\\u03A9"'], // broken currently 

// unicode with surrogate
["🍔x".charCodeAt(0), 127828],
["🍔x".charCodeAt(1), 120],
["x🍔".charCodeAt(1), 127828],
["🍔x".charAt(0), "🍔"],
["🍔x".charAt(1), "x"],
["x🍔y"[0], "x"],
["x🍔y"[1], "🍔"],
["x🍔y"[2], "y"],
["\uD83C\uDF54", "🍔"], // unicode surrogates
[JSON.parse('"\uD83C\uDF54"'), "🍔"], // unicode surrogates in JSON
[JSON.stringify('🍔'), '"\\uD83C\\uDF54"'], // Node.js doesn't escape?

[require("Storage").readJSON("utf8"), "Fön Kür Bär"],

["F\u00F6n K\u00FCr B\u00E4r".split(" ").join(","), "F\u00F6n,K\u00FCr,B\u00E4r"], // split and join

[g.wrapString("F\u00F6n K\u00FCr B\u00E4r",100).join("\n"),"F\u00F6n K\u00FCr B\u00E4r"],

["F\u00F6n"+"B\u00E4r", "F\u00F6nB\u00E4r"], // UTF8 + UTF8
["F\u00F6n"+"B\xE4r", "F\u00F6nB\u00E4r"], // UTF8 + normal
["F\xF6n"+"B\u00E4r", "F\u00F6nB\u00E4r"] // normal + UTF8

// ["😂🍔❤️🔥🥺".match(/🍔/).index, 1], // regex - needs fixing (works in most cases but returns non-UTF8 index)
];


//print(tests);
result = tests.every(t => t[0]==t[1]);

tests.forEach((t,i) => {
  if (t[0]!=t[1])
    console.log(`Test {$i} failed: ${JSON.stringify(t[1])}`);
});

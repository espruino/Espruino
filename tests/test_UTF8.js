var tests = [

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
];

//print(tests);
result = tests.every(t => t[0]==t[1]);

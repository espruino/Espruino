var hashlib = require("hashlib");

var messages = [
    "abc",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
];

var vectors = [
    /* SHA-224 */
    [ hashlib.sha224, [
        "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
        "a88cd5cde6d6fe9136a4e58b49167461ea95d388ca2bdb7afdc3cbf4",
        "c97ca9a559850ce97a04a96def6d99a9e0e0e2ab14e6b8df265fc0b3",
    ]],
    /* SHA-256 */
    [ hashlib.sha256, [
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb",
        "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1",
    ]]
];

result = 0;

vectors.forEach(function(item) {
    result |= !(item[0](messages[0]).hexdigest() == item[1][0]);
    result |= !(item[0](messages[1]).hexdigest() == item[1][1]);
    result |= !(item[0](messages[2]).hexdigest() == item[1][2]);
});

result = result == 0

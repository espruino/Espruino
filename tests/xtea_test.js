var encry=xxtea.encrypt("1234567890ABCDEFGHIJKLMN","123456");
var decry=xxtea.decrypt(encry,"123456");
//var encry=xxtea.encrypt("12345","123456");
//var decry=xxtea.decrypt(encry,"123456");
console.log({
    encry:encry,
    decry:decry
});
// Number definition from http://en.wikipedia.org/wiki/JavaScript_syntax
a = 345;    // an "integer", although there is only one numeric type in JavaScript
b = 34.5;   // a floating-point number
c = 3.45e2; // another floating-point, equivalent to 345
d = 0377;   // an octal integer equal to 255
e = 0xFF;   // a hexadecimal integer equal to 255, digits represented by the letters A-F may be upper or lowercase

result = a==345 && b*10==345 && c==345 && d==255 && e==255;

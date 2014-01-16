
result = true;

function t(x) {
 console.log(x);
 result  &= x;
}

t( 0x01 === 1);
t( 0X01 === 1);

t( 0b01 === 1);
t( 0B01 === 1);

t( 0o01 === 1);
t( 0O01 === 1);

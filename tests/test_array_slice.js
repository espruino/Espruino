var a = [1, 2, 3, 4, 5, 6];
var b = a.slice();
var c = a.slice(0);
var d = a.slice(0, 2);
var e = a.slice(2, 3);
var f = a.slice(4, 10);
var g = a.slice(10, 10);
var h = a.slice(-1, 1);
var i = a.slice(-1);
var j = a.slice(0, -1);

console.log( b !== a );

console.log( b.length === a.length );
console.log( c.length === a.length );
console.log( d.length === 2 );
console.log( e.length === 1 );
console.log( f.length === 2 );
console.log( g.length === 0 );
console.log( h.length === 0 );
console.log( i.length === 1 );
console.log( j.length === 5 );

console.log( d[0] === 1 );
console.log( d[1] === 2 );
console.log( e[0] === 3 );
console.log( f[0] === 5 );
console.log( f[1] === 6 );
console.log( i[0] === 6 );
console.log( j[0] === 1 );
console.log( j[2] === 3 );
console.log( j[4] === 5 );



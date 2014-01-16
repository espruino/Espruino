var tests=0, testpass=0;
function t(x) {
 console.log(x);
 tests++;
 if (x) testpass++;
}


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

t( b !== a );

t( b.length === a.length );
t( c.length === a.length );
t( d.length === 2 );
t( e.length === 1 );
t( f.length === 2 );
t( g.length === 0 );
t( h.length === 0 );
t( i.length === 1 );
t( j.length === 5 );

t( d[0] === 1 );
t( d[1] === 2 );
t( e[0] === 3 );
t( f[0] === 5 );
t( f[1] === 6 );
t( i[0] === 6 );
t( j[0] === 1 );
t( j[2] === 3 );
t( j[4] === 5 );

result = tests==testpass;

// https://github.com/espruino/Espruino/issues/894
// test chaining with catch...
var sequence = "";

var p = new Promise( function(resolve,reject) { reject(1); });
p.catch( function(value) {
        sequence+="A"+value;
        return value + 1;
}).then( function(value) {
        sequence+="C"+value;
        return new Promise( function( resolve ) { resolve( 4 ); } );
}).then( function( value ) {
        sequence+="D"+value;
} );
p.then(function(value) { // 1
});
setTimeout(function() {
  result = sequence == "A1C2D4";
  console.log(result, sequence);
},10);

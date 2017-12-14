// https://github.com/espruino/Espruino/issues/894
// test chaining with catch...
var sequence = "";

var p = new Promise( function(resolve,reject) { resolve(1); });
p.then( function(value) {
        sequence+="A"+value;
        return value + 1;
}).catch( function(value) {
        sequence+="C"+value;
        return new Promise( function( resolve ) { resolve( 4 ); } );
}).then( function( value ) {
        sequence+="D"+value;
} );
p.then(function(value) {
        sequence+="B"+value;
});
setTimeout(function() {
  result = sequence == "A1B1D2";
  console.log(result, sequence);
},10);

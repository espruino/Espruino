// http://forum.espruino.com/conversations/292464

// constructor function
function PengMod(a) {
      // "a" will be resolved with the method a not with the locale variable a.
      console.log( a );
      if (a !== "hello" ) throw Error("Bug!");
}
// This function masks the local variable a in the constructor!!!
PengMod.a = function () {
};
PengMod.b = 42;
PengMod.c = function () {};


try {
  new PengMod( "hello" ); 
  result = 1;
} catch (e) {
  console.log(e);
  result = 0;
}

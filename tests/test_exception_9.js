// https://github.com/espruino/Espruino/issues/1439
result = 1;
// failed
if (false) try { console.log('A'); } catch(ex) { console.log("Don't call me 1"); result = 0; }
if (false) try { throw "Foo" } catch(ex) { console.log("Don't call me 2"); result = 0; }
// works
if (false) { try { throw "Foo" } catch(ex) { console.log("Don't call me 3"); result = 0; } }
if (false) { try { console.log('A'); } catch(ex) { console.log("Don't call me 4"); result = 0; } }

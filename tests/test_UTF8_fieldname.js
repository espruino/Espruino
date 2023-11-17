// https://github.com/espruino/Espruino/issues/2429
// there were already some changes in https://github.com/espruino/Espruino/commit/dde4d3a5ca52782fbe7cefacb18d52d9655b485a

const s = "\u00FC";
const o = {};
const o2 = {"\u00FC":42}; // this one was ok after above changes

// 
print("==================== O2");
trace(o2); // ok
print("==================== O");
o[s] = 42;
trace(o); // not ok

// o an o2 were different before

result = o["\u00FC"]==42;

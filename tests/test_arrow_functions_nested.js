// https://github.com/espruino/Espruino/issues/1011
var Util = {
    foo:x=>x.map(e=>e+1)
};

trace(Util.foo);

result = Util.foo([1,2,3])=="2,3,4";

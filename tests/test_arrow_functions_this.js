global.value = 34;
var obj = {
  value : 42,
  a : function() {
    return x => this.value+x;
  },
  b : x => this.value+x
};

var r = obj.a()(8);
var r2 = obj.b(8);
result = r === 50 && r2===42;

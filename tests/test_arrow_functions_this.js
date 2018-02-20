var obj = {
  value : 42,
  a : function() {
    return x=>this.value+x;
  }
};

var r = obj.a()(8);
result = r === 50;

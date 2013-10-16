//https://github.com/espruino/Espruino/issues/99
var Class= function(){}
Class.prototype.test = function(){this.a=5}
console.log(Class.prototype)

Class.extend = function(childPrototype){
  
  var parent = this
  
  var child = function(){
    return parent.apply(this, arguments)
  };
  
  child.extend = parent.extend;
    
  var Surrogate = function() {};
  Surrogate.prototype = parent.prototype;
//  child.prototype = new Surrogate; // this line breaks it
  child.prototype = new Surrogate();
  
  for(var key in childPrototype){
    child.prototype[key] = childPrototype[key];
  }
  return child
}

var T = Class.extend({
  x : 5,
  test: function (){this.a=8}
})

a=new Class()
b=new T()

var as = ""+a.test;
var bs = ""+b.test;
print(as);
print(bs);

result = as=="function () {this.a=5}" && bs=="function () {this.a=8}";

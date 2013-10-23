function A() { 
  this.foo = 42;
}
A.prototype.getThis = function() {
  return this;
}
A.prototype.b = function() {
  return this.getThis.apply(this);
}


var a = new A();
var r = [];
console.log(r[0]=JSON.stringify(a)); 
console.log(r[1]=JSON.stringify(a.getThis()));
console.log(r[2]=JSON.stringify(a.b()));

result = r[0]=='{"foo":42}' && r[1]=='{"foo":42}' && r[2]=='{"foo":42}';

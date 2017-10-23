function Foo() {
  this.bar();
}
Foo.prototype.bar = function() {
  result=1;
};
new Foo();

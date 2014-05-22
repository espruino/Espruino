function foo() {
  this.bar = new Bar(this);
  console.log(this); // may assert fail
  result = 1;
}
function Bar( fooThis ) {
   this.fooThis = fooThis; 
}
var f = new foo();

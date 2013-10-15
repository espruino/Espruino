function Class() {                          
  this.test = "test";
}
Class.prototype.initialize = function(){
    console.log("initialize");
}
Class.prototype.getTest = function(){
    return this.test;
}

var toto = new Class();

result = toto.test == "test";

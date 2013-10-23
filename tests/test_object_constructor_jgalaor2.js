// https://github.com/espruino/Espruino/issues/109

var Class = function(){
  this.initialize.apply(this, arguments);
};

Class.prototype.initialize = function(){}

Class.extend = function(childPrototype){
  
  var parent = this;
  
  var child = function(){
    return parent.apply(this, arguments);
  };
  
  child.extend = parent.extend;
  
  var Surrogate = function() {};
  
  Surrogate.prototype = parent.prototype;
  
  child.prototype = new Surrogate();
 
  for(var key in childPrototype){
    child.prototype[key] = childPrototype[key];
  }
  return child;
};

var Toto = Class.extend({   
  initialize : function(){
    this.test = 5;
    console.log(a=JSON.stringify(this));
  },

  getTest : function(){
    trace(this);
    console.log(b=JSON.stringify(this));
  }
});

var t = new Toto(); 
// print {"test":5}

t.getTest();
// print {"test":5}

result = a=='{"test":5}' && b=='{"test":5}';

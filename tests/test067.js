// using 'this' in callback
result = 0;
Pin.prototype.foo = function() { this.set(); setTimeout(function() { print(this); this.reset(); result=1; }, 10); } 
LED1.foo()

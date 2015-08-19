// https://github.com/espruino/Espruino/issues/533

var s = 0;

function timer() {
  var t=0;
    return {
      start: function() {
        s++;
        console.log(t++);
        if (s>5) 
          result = t==6;
        else
          setTimeout(this.start.bind(this), 1);
           
      }
    };
  }


var signal = timer();
signal.start();


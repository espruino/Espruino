var history = [324,64,6473,3754,7543];

function draw() {
  var lastx = 0;
  var lasty = 240;
  for (idx in history) {
    var thisx = idx*LCD.WIDTH/history.length;
    var thisy = 240 - history[idx]*2;  
    // ...
    lastx = thisx. // this would have caused an assert fail
    lasty = thisy;    
  }
}

setTimeout("result = 1;",10);
draw();




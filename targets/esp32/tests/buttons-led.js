console.log('Button and GPIO tests');
// Led on D22 should flash once per second
// [boot] button on board should toggle led state
// Any one wire devices on D23 should be listed.
var led = Pin(D23); 
var led = Pin(D22);
//var led = Pin(D15); //
//

var toggle=1;


//pinMode(F5, "input");
 digitalWrite(D23, 0);
pinMode(D23, "input");
var ow = new OneWire(D23);


console.log('look for one wire');
var sensors = ow.search().map(function (device) {
  console.log(device);
  return require("DS18B20").connect(ow, device);
});

console.log(sensors);


function updateLed(){
  digitalWrite(led, toggle);
  toggle=!toggle;
}

// Boot button   1 up 0 down
digitalRead(D0);

function button_down() {
  updateLed();
  print('button');
}

setWatch(button_down,D0, {repeat:true, edge:"rising"});

setInterval( function() {
  //console.log('tick');
  updateLed();
}, 1000 );

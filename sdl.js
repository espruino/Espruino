// --- Linux - Bangle glue

function bangle_project(latlong) {
  let degToRad = Math.PI / 180; // degree to radian conversion              
  let latMax = 85.0511287798; // clip latitude to sane values          
  let R = 6378137; // earth radius in m                                
  let lat = latlong.lat;
  let lon = latlong.lon;
  if (lat > latMax) lat=latMax;
  if (lat < -latMax) lat=-latMax;
  let s = Math.sin(lat * degToRad);
  let o = {}
  o.x = R * lon * degToRad;
  o.y = R * Math.log((1 + s) / (1 - s)) / 2;
  print("Project", latlong, o);    
  return o;
}

var bangle_on_map = {}

function bangle_on(event, callback) {
  bangle_on_map[event] = callback;
}

function bangle_setUI(map) {
  if (map.drag) {
    bangle_on_map['drag'] = map.drag;
  }
}

function initWindow(x, y) {
  Bangle.appRect = [ 0, 0, x, y ];
  g = Graphics.createSDL(x, y, 16);
  g.setColor(1,1,1);
  g.fillRect(0, 0, x, y);
  g.flip = print;
}

Bangle = {};
Bangle.setGPSPower = print;
Bangle.loadWidgets = print;
Bangle.drawWidgets = print;
Bangle.setUI =  bangle_setUI;
Bangle.project = bangle_project;
Bangle.isCharging = function () { return false; }
Bangle.on = bangle_on;
WIDGETS = false;
E = {};
E.getBattery = function () { return 100; }
//initWindow(1024, 768);
initWindow(240, 240);

function backdoor(x, y) { return peek8(x); }
//function backdoor(x, y) { return g.getPixel(x, 0); }

function sdl_drag(is_down) {
  let drag = {}
  drag.b = is_down;
  drag.x = backdoor(5,0);
  drag.y = backdoor(6.0);
  print("...mouse down", drag.x, drag.y);
  let d = bangle_on_map['drag'];
  if (d) {
    d(drag);
  }
}

var sdl_is_down = false;

function sdl_key(key) {
  switch(key) {
  case 65:
    break;
  }
  let d = bangle_on_map['key'];
  if (d) {
    d(key);
  }
}

function sdl_poll() {
  e = backdoor(0, 0);
  while (e) {
    type = backdoor(1, 0);
    switch(type) {
    case 1: //print("...window in?");
      break;
    case 2:
      let key = backdoor(2, 0);
      print("...key down", key);
      sdl_key(key);
      break;
    case 3: print("...key up"); break;
    case 4:
      if (sdl_is_down) {
	print("...move");
	sdl_drag(true);
      }
      break;
    case 5:
      sdl_is_down = true;
      sdl_drag(true);
      break;
    case 6: sdl_is_down = false; sdl_drag(false); print("...mouse up"); break;
    case 12: print("...exit"); quit(); break;
    default: print("...type:", type); break;
    }
    e = backdoor(0, 0);
  }
}

print("Test being loaded");
setInterval(sdl_poll, 10);

// --- end glue

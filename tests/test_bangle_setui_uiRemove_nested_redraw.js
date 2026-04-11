// Regression test for nested Bangle.setUI() from uiRemove.
// A redraw-capable replacement UI installed during uiRemove must win over
// the stale outer setUI() that triggered the removal.

var Bangle = {
  on : function() {},
  prependListener : function() {},
  removeListener : function() {},
  haptic : function() {},
  appRect : { y : 0 }
};
var g = {
  reset : function() { return this; },
  clearRect : function() { return this; },
  setColor : function() { return this; },
  drawImage : function() { return this; }
};
var BTN1 = 1;
function setWatch() { return {}; }
function clearWatch() {}

Bangle.setUI = eval(require("fs").readFileSync("libs/js/banglejs/Bangle_setUI_Q3.js").toString());

function nestedDraw() {}
function staleOuterDraw() {}

Bangle.setUI({
  mode : "custom",
  remove : function() {
    Bangle.setUI({
      mode : "custom",
      redraw : nestedDraw
    });
  }
});

Bangle.setUI({
  mode : "updown",
  redraw : staleOuterDraw
}, function() {});

result = Bangle.uiRedraw === nestedDraw;

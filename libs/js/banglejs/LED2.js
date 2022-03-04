(function() { // by @allObjects
  return { 
    _v: false, // status on / off
    read: function() { return this._v; },
    set: function() { this.write(1); },
    reset: function() { this.write(0); },
    write: function(v) { g.setColor((this._v=!!v)?"#0f0":g.theme.bg).fillCircle((g.getWidth()+20)/2,5,5);Bangle.setLCDPower(1); },
    toggle: function() { this.write(!this._v); }
  };
})

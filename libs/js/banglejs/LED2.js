(function() { // by @allObjects
  return { 
    read: function() { return !!global["\xff"].LED2; },
    set: function() { this.write(1); },
    reset: function() { this.write(0); },
    write: function(v) { g.setColor((global["\xff"].LED2=!!v)?"#0f0":g.theme.bg).fillCircle((g.getWidth()+20)/2,5,5);Bangle.setLCDPower(1); },
    toggle: function() { this.write(!this.read()); }
  };
})

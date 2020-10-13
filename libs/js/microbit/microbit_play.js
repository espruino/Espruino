(function(waveform, pitch, callback) {
  if (!isFinite(pitch)) pitch=4000;
  if (!Microbit.sounds) this.sounds=0;
  if (Microbit.sounds>2) throw new Error("Too many sounds playing at once");
  var w = new Waveform(waveform.length);
  w.buffer.set(waveform);
  w.on("finish", function(buf) {
    Microbit.sounds--;
    if (!Microbit.sounds) { /* TODO: stop output */ }
    if (callback) callback();
  });
  if (!Microbit.sounds) {
    analogWrite(Microbit.SPEAKER, 0.5, {freq:40000});
  }
  Microbit.sounds++;
  w.startOutput(Microbit.SPEAKER, pitch);
})

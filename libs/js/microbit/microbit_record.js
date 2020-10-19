(function(pitch, callback, length) {
  if (!isFinite(length)) length=6000;
  var w = new Waveform(length);
  w.on("finish", function(buf) {
    Microbit.MIC_ENABLE.reset();
    if (callback) callback(w.buffer);
  });
  Microbit.MIC_ENABLE.set();
  w.startInput(Microbit.MIC, pitch);
})

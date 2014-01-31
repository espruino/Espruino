var pos=0;
function getPattern() {
  var rgb = "";
  pos++;
  for (var i=0;i<75;i+=3) {
     rgb += String.fromCharCode((1 + Math.sin((i+pos)*0.1324)) * 127) +
            String.fromCharCode((1 + Math.sin((i+pos)*0.1654)) * 127) +
            String.fromCharCode((1 + Math.sin((i+pos)*0.1)) * 127);
  }
}

for (var i=0;i<100;i++) getPattern();

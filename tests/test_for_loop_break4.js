// http://forum.espruino.com/conversations/364276/#comment16017216
var lvls = [65535,54038,47824,39322,32768,26526,18388,11818,5958],
    halfDiff = 2979; //2958/2
function anaGetKey(samples){
  var keys = samples.map(function(v)  {
    var i;
    var val = v + halfDiff;
    for (i=0; i<lvls.length; i++)    {
      if (lvls[i] < val){
        console.log("found : "+i+" for "+v+" - "+val);
        break;
      }
    }
    console.log(" => return "+i);
    return i;
  });
  console.log(keys);
  return keys;
}
result = anaGetKey([6000,12000,54000,65000]) == "8,7,1,0";
//console.log(result);

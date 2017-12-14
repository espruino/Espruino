// https://github.com/espruino/Espruino/issues/984

var obja={};
obja["1233456789012345"]="yes";
var ra = JSON.stringify(obja);

var objb={};
objb["1233456789012345"]=true;
var rb = JSON.stringify(objb);


result = ra=="{\"1233456789012345\":\"yes\"}" &&
         rb=="{\"1233456789012345\":true}";

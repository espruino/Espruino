
var r = [
  (new Array()).toString() == "",
  (new Array(2)).toString() == ",",
  (new Array(3)).toString() == ",,",
  (new Array(3.0)).toString() == ",,",
  (new Array(Math.ceil(3.2134567))).toString() == ",,,",
  (new Array("3")).toString() == "3",
];



try {
  new Array(2.1);
  console.log("New array with FP value should have thrown an error");
} catch (e) {
  result=r.every(function(s){return s;});
}

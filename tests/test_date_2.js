// https://github.com/espruino/Espruino/issues/408

function dump( d ) {
  console.log(" toString: ", d.toString());
  console.log(" Year:     ", d.getFullYear());
  console.log(" Month:    ", d.getMonth() + 1);
  console.log(" Date:     ", d.getDate());
  console.log(" Hours:    ", d.getHours());
  console.log(" Mins:     ", d.getMinutes());
  console.log(" Secs:     ", d.getSeconds());
  console.log(" Unixtime: ", d.getTime());
}

var da = new Date( 1406481531000 );
var db = new Date(
        da.getFullYear(),
        da.getMonth(),
        da.getDate(),
        da.getHours(),
        da.getMinutes(),
        da.getSeconds(),
        0
);
var dc = new Date( 2014, 6, 27, 17, 18, 51, 0 );

console.log("DA");
dump(da);
console.log("DB");
dump(db);
console.log("DC");
dump(dc);

var expected = "Sun Jul 27 2014 17:18:51 GMT+0000";
var das = da.toString();
var dbs = db.toString();
var dcs = dc.toString();
result = das==expected && dbs==expected && dcs==expected;

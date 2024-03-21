function round(n, dp) {
  if (dp===undefined) dp=1;
  var p = Math.min(dp,dp - Math.floor(Math.log(n)/Math.log(10)));
  return n.toFixed(p);
}
exports = { name : "system", currencySym:"£",
  translate : str=>str, // as-is
  date : (d,short) => short?("0"+d.getDate()).substr(-2)+"/"+("0"+(d.getMonth()+1)).substr(-2)+"/"+d.getFullYear():d.toString().substr(4,11).trim(), // Date to "Feb 28 2020" or "28/02/2020"(short)
  time : (d,short) => { // Date to  "4:15.28 pm" or "15:42"(short)
	  var h = d.getHours(), m = d.getMinutes()
    if ((require('Storage').readJSON('setting.json',1)||{})["12hour"])
      h = (h%12==0) ? 12 : h%12; // 12 hour
    if (short)
      return (" "+h).substr(-2)+":"+("0"+m).substr(-2);
    else {
      var r = "am";
      if (h==0) { h=12; }
      else if (h>=12) {
        if (h>12) h-=12;
        r = "pm";
      }
      return (" "+h).substr(-2)+":"+("0"+m).substr(-2)+"."+("0"+d.getSeconds()).substr(-2)+" "+r;
    }
  },
  dow : (d,short) => "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday".split(",")[d.getDay()].substr(0, short ? 3 : 10), // Date to "Monday" or "Mon"(short)
  month : (d,short) => "January,February,March,April,May,June,July,August,September,October,November,December".split(",")[d.getMonth()].substr(0, short ? 3 : 10), // Date to "February" or "Feb"(short)
  number : n => n.toString(), // more fancy?
  currency : n => "£"+n.toFixed(2), // number to "£1.00"
  distance : (m,dp) => (m<1000)?round(m,dp)+"m":round(m/1000,dp)+"km", // meters to "123m" or "1.2km" depending on size
  speed : (s,dp) => round(s/1.60934,dp)+"mph",// kph to "123mph"
  temp : (t,dp) => round(t,dp)+"'C", // degrees C to degrees C
  meridian: d => (d.getHours() <= 12) ? "am":"pm" // Date to am/pm
};

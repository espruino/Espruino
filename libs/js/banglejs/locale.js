exports = { name : "en_GB", currencySym:"£",
  translate : str=>str, // as-is
  date : (d,short) => short?("0"+d.getDate()).substr(-2)+"/"+("0"+(d.getMonth()+1)).substr(-2)+"/"+d.getFullYear():d.toString().substr(4,11), // Date to "Feb 28 2020" or "28/02/2020"(short)
  time : (d,short) => { // Date to  "4:15.28 pm" or "15:42"(short)
    if (short)
      return d.toString().substr(16,5);
    else {
      var h = d.getHours(), m = d.getMinutes(), r = "am";
      if (h==0) { h=12; }
      else if (h>=12) {
        if (h>12) h-=12;
        r = "pm";
      }
      return (" "+h).substr(-2)+":"+("0"+m).substr(-2)+"."+("0"+d.getSeconds()).substr(-2)+" "+r;
    }
  },
  dow : (d,short) => short?d.toString().substr(0,3):"Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday".split(",")[d.getDay()], // Date to "Monday" or "Mon"(short)
  month : (d,short) => short?d.toString().substr(4,3):"January,February,March,April,May,June,July,August,September,October,November,December".split(",")[d.getMonth()], // Date to "February" or "Feb"(short)
  number : n => n.toString(), // more fancy?
  currency : n => "£"+n.toFixed(2), // number to "£1.00"
  distance : m => (m<1000)?Math.round(m)+"m":Math.round(m/160.934)/10+"mi", // meters to "123m" or "1.2mi" depending on size
  speed : s => Math.round(s)+"mph",// kph to "123mph"
  temp : t => Math.round(t)+"'C" // degrees C to degrees C
};

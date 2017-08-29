pass=0;

var gmt = [
[ Date.parse("2011-10-20") , 1319068800000.0 ],
[ Date.parse("2011-10-20T14:48:12.345") , 1319122092345.0 ],
[ Date.parse("Aug 9, 1995") , 807926400000.0 ],
[ new Date("Wed, 09 Aug 1995 00:00:00").getTime() , 807926400000.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT") , 0.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT-0100") , 3600000.0 ],
[ Date.parse("Mon, 25 Dec 1995 13:30:00 +0430"), 819882000000.0 ],
[ new Date(807926400000.0).toString() , "Wed Aug 9 1995 00:00:00 GMT+0000" ],
[ new Date("Fri, 20 Jun 2014 15:27:22 GMT").toString(), "Fri Jun 20 2014 15:27:22 GMT+0000"],
[ new Date("Fri, 20 Jun 2014 15:27:22 GMT").toISOString(), "2014-06-20T15:27:22.000Z"],
[ new Date("Fri, 20 Jun 2014 17:27:22 GMT+0200").toISOString(), "2014-06-20T15:27:22.000Z"]
];

gmt.forEach(function(n) { if (n[0]==n[1]) pass++; });

E.setTimeZone(2);
var cest = [
[ Date.parse("2011-10-20") , 1319068800000.0 ],
[ Date.parse("2011-10-20T14:48:12.345") , 1319114892345.0 ],
[ Date.parse("Aug 9, 1995") , 807919200000.0 ],
[ new Date("Wed, 09 Aug 1995 00:00:00").getTime() , 807919200000.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT") , 0.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT-0100") , 3600000.0 ],
[ Date.parse("Mon, 25 Dec 1995 13:30:00 +0430"), 819882000000.0 ],
[ new Date(807926400000.0).toString() , "Wed Aug 9 1995 02:00:00 GMT+0200" ],
[ new Date("Fri, 20 Jun 2014 15:27:22 GMT").toString(), "Fri Jun 20 2014 17:27:22 GMT+0200"],
[ new Date("Fri, 20 Jun 2014 15:27:22 GMT").toISOString(), "2014-06-20T15:27:22.000Z"],
[ new Date("Fri, 20 Jun 2014 17:27:22 GMT+0200").toISOString(), "2014-06-20T15:27:22.000Z"]
];

cest.forEach(function(n) { if (n[0]==n[1]) pass++; });

result = pass == gmt.length + cest.length;

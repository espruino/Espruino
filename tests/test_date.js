var r = [
[ Date.parse("2011-10-20") , 1319068800000.0 ],
[ Date.parse("2011-10-20T14:48:12.345") , 1319122092345.0 ],
[ Date.parse("Aug 9, 1995") , 807926400000.0 ],
[ new Date("Wed, 09 Aug 1995 00:00:00").getTime() , 807926400000.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT") , 0.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT-0100") , 3600000.0 ],
[ Date.parse("Mon, 25 Dec 1995 13:30:00 +0430"), 819882000000.0 ],
[ new Date(807926400000.0).toString() , "Wed Aug 9 1995 00:00:00 GMT+0000" ],
[ new Date("Fri, 20 Jun 2014 15:27:22 GMT").toString(), "Fri Jun 20 2014 15:27:22 GMT+0000"]
];

pass=0;
r.forEach(function(n) { if (n[0]==n[1]) pass++; });
result = pass == r.length;

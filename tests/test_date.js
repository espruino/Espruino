var r = [
[ Date.parse("2011-10-20") , 1319068800000.0 ],
[ Date.parse("2011-10-20T14:48:12.345") , 1319122092345.0 ],
[ Date.parse("Aug 9, 1995") , 807926400000.0 ],
[ new Date("Wed, 09 Aug 1995 00:00:00").getTime() , 807926400000.0 ],
[ Date.parse("Thu, 01 Jan 1970 00:00:00 GMT") , 0.0 ],
[ new Date(807926400000.0).toString() , "Wed Aug 9 1995 00:00:00 GMT+0000" ],
];

pass=0;
r.forEach(function(n) { if (n[0]==n[1]) pass++; });
result = pass == r.length;

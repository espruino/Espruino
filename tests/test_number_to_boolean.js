var r = [
!!5, true,
!!1, true,
!!-1, true,
!!0, false,
!!0.0, false,
!!0.1, true,
!!-0.01, true,
!!NaN, false,
!!Infinity, true,
];

result = 1;
for (i=0;i<r.length;i+=2)
  if (r[i]!=r[i+1])
    result=0;



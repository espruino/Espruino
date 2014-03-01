// interrupt in interval

result = 0;
print("ONE interrupt expected here");

setInterval(function () {
  result++;
  if (result>10) clearInterval();
  interrupt(); // force an interrupt
},1);

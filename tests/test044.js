digitalRead(["D12","D13","D14","D15"]);

var n=1; 
function f() { 
  print(n);
  n=n<<1; 
  if (n>15) n=1; 
  digitalWrite(["D12","D13","D14","D15"],n); 
  if (n==1) clearInterval(timer);
}; 

var timer = setInterval(f,20);

result = 1;

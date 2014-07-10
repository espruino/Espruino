// simple test that heavily minified version of stepper driver doesn't cause errors
function stepper(f,g,h,k){var a,b,d,e,c;a=0;b=h?[1,2,4,8]:[1,3,2,6,4,12,8,9];d=1E3/g;c=k;this.start=function(){e=setInterval(function(){c?(a++,a%=b.length):(a--,0>a&&(a=b.length-1));digitalWrite(f,b[a]);result=1;},d)};this.stop=function(){clearInterval(e)};this.changeDirection=function(a){this.stop();c=a;this.start()};this.start()}var st=new stepper([D0,D1,D2,D3],100,!0,!0);
setTimeout("st.stop();",10);


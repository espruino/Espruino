
result=1;

function test(a,b) {
  if (JSON.stringify(a)!=JSON.stringify(b)) {
    console.log("FAIL",a,"vs",b);
    result = 0;
  }
}

test(url.parse("/hello?a=b"), {"method":"GET","host":"","path":"/hello?a=b","pathname":"/hello","search":"?a=b","port":80,"query":"a=b"})
test(url.parse("/hello?a=b&c=dd",false), {"method":"GET","host":"","path":"/hello?a=b&c=dd","pathname":"/hello","search":"?a=b&c=dd","port":80,"query":"a=b&c=dd"})
test(url.parse("/hello?a=b&c=dd",true), {"method":"GET","host":"","path":"/hello?a=b&c=dd","pathname":"/hello","search":"?a=b&c=dd","port":80,"query":{"a":"b","c":"dd"}})

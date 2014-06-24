var a = url.parse("/a?1=2&3=4",true).query;
result = a[1]==2 && a[3]==4;


// From TiCPU, http://forum.espruino.com/conversations/568

// Non-nested quick sort - minified
Array.prototype.nqsort=function(d){var f,c=0,a,b;void 0===d&&(d=parseInt(Math.floor(this.length/5),10));var g=new Uint16Array(d),e=new Uint16Array(d);g[0]=0;for(e[0]=this.length;0<=c;)if(a=g[c],b=e[c]-1,a<b){f=this[a];if(c===d-1)return!1;for(;a<b;){for(;this[b]>=f&&a<b;)b--;for(a<b&&(this[a++]=this[b]);this[a]<=f&&a<b;)a++;a<b&&(this[b--]=this[a])}this[a]=f;g[c+1]=a+1;e[c+1]=e[c];e[c++]=a}else c--;return!0};
Uint16Array.prototype.nqsort = Array.prototype.nqsort;

// Tests
var tQsort = new Uint16Array([5454,5449,5380,5412,5380,5366,5344,5395,5398,5424,5422,5473,5420,5432,5376,5354,5561,5288,5393,5388,5422,5427,5476,5407,5385,5180,5363,5324,5395,5393,5410,5405,5349,5361,5385,5412,5373,5373,5478,5420,5446,5395,5339,5407,5420,5356,5336,5427,5459,5378,5336,5349,5420,5405,5434,5383,5446,5422,5349,5329,5405,5434,5446,5336,5427,5473,5402,5170,5388,5412,5456,123,456,789]);
tQsort.nqsort();

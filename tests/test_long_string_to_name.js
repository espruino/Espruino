o = {};
n = 'a';
i = 1;
while (true) {
  o[n] = i++;
  n += (i % 10) || (i / 10);
  if (E.getSizeOf(n) == 3) break;
}
/* o.a == 1
 * o.a2 == 2
 * ..
 * o.a234567891 == 10
 * o.a2345678911 == 11
 * ..
 */
while (true) {
  n = n.substring(0, n.length - 1);
  if (!n.length) {
    result = true;
    break;
  }
  if (o[n] != n.length) {
    result = false;
    break;
  }
}

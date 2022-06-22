/**

Temporary file by Deirdre O'Byrne for Daylight Savings Calculations

*/

#include <stdio.h>
#include <stdlib.h>

int dayNumber(int y, int m, int d) {
  int ans;

  if (m < 2) {
    y--;
    m+=12;
  }
  ans = (y/100);
  ans = 365*y + (y>>2) - ans + (ans>>2) + 30*m + ((3*m+6)/5) + d - 719531;
  return ans;
}

int dstChangeDay(int y, int dow_number, int month, int dow, int day_offset) {
  int m = month;
  int ans;
  if (dow_number == 4) { // last X of this month? Work backwards from 1st of next month.
    if (++m > 11) {
      y++;
      m-=12;
    }
  }
  ans = dayNumber(y, m, 1); // ans % 7 is 0 for THU; (ans + 4) % 7 is 0 for SUN
  if (dow_number == 4) {
    ans -= 7 - (7 - ((ans + 4) % 7) + dow) % 7;
  } else {
    ans += 7 * dow_number + (14 + dow - ((ans + 4) % 7)) % 7;
  }
  ans -= day_offset;
  return ans;
}

void getDate(int day, int *y, int *m, int *date) {
  int a = day + 135081;
  int b,c,d,e;
  a = (a-(a/146097)+146095)/36524;
  a = day + a - (a>>2);
  c = ((a<<2)+2877911)/1461;
  d = 365*c + (c>>2);
  b = a + 719600 - d;
  e = (5*b-1)/153;
  *date=b-30*e-((3*e)/5);
  if (e<14)
    *m=e-1;
  else
    *m=e-13;
  if (e>13)
    *y=c+1;
  else
    *y=c;
}

int main(int argc, char *argv[]) {
  int yr,y,m,d,day;
  yr=atoi(argv[1]);
  day=dstChangeDay(yr,4,2,0,0);
  getDate(day,&y,&m,&d);
  printf("Start : %04d/%02d/%02d  ",y,m,d);
  day=dstChangeDay(yr,4,9,0,0);
  getDate(day,&y,&m,&d);
  printf("End : %04d/%02d/%02d\n",y,m,d);
}



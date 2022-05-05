/**
 * The $1 Unistroke Recognizer (JavaScript version)
 *
 *  Jacob O. Wobbrock, Ph.D.
 *  The Information School
 *  University of Washington
 *  Seattle, WA 98195-2840
 *  wobbrock@uw.edu
 *
 *  Andrew D. Wilson, Ph.D.
 *  Microsoft Research
 *  One Microsoft Way
 *  Redmond, WA 98052
 *  awilson@microsoft.com
 *
 *  Yang Li, Ph.D.
 *  Department of Computer Science and Engineering
 *  University of Washington
 *  Seattle, WA 98195-2840
 *  yangli@cs.washington.edu
 *
 * The academic publication for the $1 recognizer, and what should be
 * used to cite it, is:
 *
 *     Wobbrock, J.O., Wilson, A.D. and Li, Y. (2007). Gestures without
 *     libraries, toolkits or training: A $1 recognizer for user interface
 *     prototypes. Proceedings of the ACM Symposium on User Interface
 *     Software and Technology (UIST '07). Newport, Rhode Island (October
 *     7-10, 2007). New York: ACM Press, pp. 159-168.
 *     https://dl.acm.org/citation.cfm?id=1294238
 *
 * The Protractor enhancement was separately published by Yang Li and programmed
 * here by Jacob O. Wobbrock:
 *
 *     Li, Y. (2010). Protractor: A fast and accurate gesture
 *     recognizer. Proceedings of the ACM Conference on Human
 *     Factors in Computing Systems (CHI '10). Atlanta, Georgia
 *     (April 10-15, 2010). New York: ACM Press, pp. 2169-2172.
 *     https://dl.acm.org/citation.cfm?id=1753654
 *
 * This software is distributed under the "New BSD License" agreement:
 *
 * Copyright (C) 2007-2012, Jacob O. Wobbrock, Andrew D. Wilson and Yang Li.
 * All rights reserved. Last updated July 14, 2018.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of the University of Washington nor Microsoft,
 *      nor the names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Jacob O. Wobbrock OR Andrew D. Wilson
 * OR Yang Li BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

/* TODO for Bangle.js

 * Move 'points' to an array of int8
 *

 */

#include <math.h>
#include <stdint.h>
#include <float.h>
#include <alloca.h>
#include "jsinteractive.h"
#define NUMPOINTS 32
#define SQUARESIZE 176
#define PI 3.141592f
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// DollarRecognizer constants
//

const float Diagonal = sqrtf(SQUARESIZE * SQUARESIZE + SQUARESIZE * SQUARESIZE);
const float AngleRange = 45.0f * PI / 180.0f;
const float AnglePrecision = 2.0f * PI / 180.0f;
const float Phi = 0.5f * (-1.0f + sqrtf(5.0f)); // Golden Ratio

//
// Point class
//
typedef struct {
  float X,Y;
} Point;
typedef struct {
  float X,Y,width,height;
} Rectangle;
typedef struct {
  Point points[NUMPOINTS];
} Unistroke;

void Resample(Point *dst, int n, Point *points, int pointsLen);
float IndicativeAngle(Point *points, int pointsLen);
void RotateBy(Point *dst, Point *points, int pointsLen, float radians);
void ScaleTo(Point *dst, Point *points, int pointsLen, float size);
void TranslateTo(Point *dst, Point *points, int pointsLen, Point pt);
void Vectorize(float *vector /* pointsLen*2 */, Point *points, int pointsLen);
float OptimalCosineDistance(float *v1, float *v2, int vLen);
float DistanceAtBestAngle(Point *points, int pointsLen, Point *T, float a, float b, float threshold);
float DistanceAtAngle(Point *points, int pointsLen, Point *T, float radians);
Point Centroid(Point *points, int pointsLen);
Rectangle BoundingBox(Point *points, int pointsLen);
float PathDistance(Point *pts1, Point *pts2, int pointsLen);
float PathLength(Point *points, int pointsLen);
float Distance(Point p1, Point p2);
float Deg2Rad(float d);

/*void dumpPts(const char *n, Point *points, int pointCount) {
  jsiConsolePrintf("%s\n",n);
  for (int i=0;i<pointCount;i++) {
    jsiConsolePrintf(" %d: %f,%f\n", i, points[i].X, points[i].Y);
  }
}*/

// A unistroke template -
Unistroke newUnistroke(Point *points, int pointCount) {
  Unistroke t;
  Resample(t.points, NUMPOINTS, points, pointCount);
  float radians = IndicativeAngle(t.points, NUMPOINTS);
  // GW: 'snap' rotation to nearest 90 degrees
  while (radians < -PI/4) radians += PI/2;
  while (radians > PI/4) radians -= PI/2;
  RotateBy(t.points, t.points, NUMPOINTS, -radians);
  ScaleTo(t.points, t.points, NUMPOINTS, SQUARESIZE);
  Point Origin = {0,0};
  TranslateTo(t.points, t.points, NUMPOINTS, Origin);
  //t.Vector = Vectorize(this.Points); // for Protractor
  return t;
}


void uint8ToPoints(Point *points, const uint8_t *xy, int xyCount) {
  for (int i=0;i<xyCount;i++) {
    points[i].X = xy[i*2];
    points[i].Y = xy[i*2 + 1];
  }
}

Unistroke newUnistroke8(const uint8_t *xy, int xyCount) {
  Point points[NUMPOINTS];
  uint8ToPoints(points, xy, xyCount);
  return newUnistroke(points, xyCount);
}

//
// Private helper functions from here on down
//
void Resample(Point *dst, int n, Point *points, int pointsLen) // points is damaged by calling Resample
{
  float I = PathLength(points, pointsLen) / (n - 1.0f); // interval length
  float D = 0.0;
  int dstLen = 0;
  dst[dstLen++] = points[0];
  for (int i = 1; i < pointsLen; i++)
  {
    float d = Distance(points[i-1], points[i]);
    if ((D + d) >= I) {
      float qx = points[i-1].X + ((I - D) / d) * (points[i].X - points[i-1].X);
      float qy = points[i-1].Y + ((I - D) / d) * (points[i].Y - points[i-1].Y);
      Point q = {qx, qy};
      dst[dstLen++] = q; // append new point 'q'
      points[i-1]=q;i--; // insert 'q' at position i in points s.t. 'q' will be the next i
      D = 0.0;
    } else D += d;
  }
  if (dstLen == n - 1) {// sometimes we fall a rounding-error short of adding the last point, so add it if so
    Point q = {points[pointsLen- 1].X, points[pointsLen - 1].Y };
    dst[dstLen++] = q;
  }
}
float IndicativeAngle(Point *points, int pointsLen)
{
  Point c = Centroid(points, pointsLen);
  return (float)atan2(c.Y - points[0].Y, c.X - points[0].X);
}
void RotateBy(Point *dst, Point *points, int pointsLen, float radians) // rotates points around centroid
{
  Point c = Centroid(points, pointsLen);
  float fcos = (float)cos(radians);
  float fsin = (float)sin(radians);
  for (int i = 0; i < pointsLen; i++) {
    dst[i].X = (points[i].X - c.X) * fcos - (points[i].Y - c.Y) * fsin + c.X;
    dst[i].Y = (points[i].X - c.X) * fsin + (points[i].Y - c.Y) * fcos + c.Y;
  }
}
void ScaleTo(Point *dst, Point *points, int pointsLen, float size) // non-uniform scale; assumes 2D gestures (i.e., no lines)
{
  Rectangle B = BoundingBox(points, pointsLen);
  // GW: stop us rescaling so the aspect ratio it toally wrong
  // eg. a line
  if (B.width<B.height/2) B.width = B.height/2;
  if (B.height<B.width/2) B.height = B.width/2;
  // not rescale
  for (int i = 0; i < pointsLen; i++) {
    dst[i].X = points[i].X * (size / B.width);
    dst[i].Y = points[i].Y * (size / B.height);
  }
}
void TranslateTo(Point *dst, Point *points, int pointsLen, Point pt) // translates points' centroid
{
  Point c = Centroid(points, pointsLen);
  for (int i = 0; i < pointsLen; i++) {
    dst[i].X = points[i].X + pt.X - c.X;
    dst[i].Y = points[i].Y + pt.Y - c.Y;
  }
}
void Vectorize(float *vector /* pointsLen*2 */, Point *points, int pointsLen) // for Protractor
{
  float sum = 0.0;
  for (int i = 0; i < pointsLen; i++) {
    vector[i*2] = points[i].X;
    vector[i*2+1] = points[i].Y;
    sum += points[i].X * points[i].X + points[i].Y * points[i].Y;
  }
  float magnitude = sqrtf(sum);
  for (int i = 0; i < pointsLen*2; i++)
    vector[i] /= magnitude;
}
float OptimalCosineDistance(float *v1, float *v2, int vLen) // for Protractor
{
  float a = 0.0;
  float b = 0.0;
  for (int i = 0; i < vLen; i += 2) {
    a += v1[i] * v2[i] + v1[i+1] * v2[i+1];
    b += v1[i] * v2[i+1] - v1[i+1] * v2[i];
  }
  float angle = (float)atan(b / a);
  return (float)acos(a * cos(angle) + b * sin(angle));
}
float DistanceAtBestAngle(Point *points, int pointsLen, Point *T, float a, float b, float threshold)
{
  float x1 = Phi * a + (1.0f - Phi) * b;
  float f1 = DistanceAtAngle(points, pointsLen, T, x1);
  float x2 = (1.0f - Phi) * a + Phi * b;
  float f2 = DistanceAtAngle(points, pointsLen, T, x2);
  while (fabs(b - a) > threshold)
  {
    if (f1 < f2) {
      b = x2;
      x2 = x1;
      f2 = f1;
      x1 = Phi * a + (1.0f - Phi) * b;
      f1 = DistanceAtAngle(points, pointsLen, T, x1);
    } else {
      a = x1;
      x1 = x2;
      f1 = f2;
      x2 = (1.0f - Phi) * a + Phi * b;
      f2 = DistanceAtAngle(points, pointsLen, T, x2);
    }
  }
  return MIN(f1, f2);
}
float DistanceAtAngle(Point *points, int pointsLen, Point *T, float radians)
{
  Point *newpoints = (Point*)alloca(sizeof(Point)*pointsLen);
  RotateBy(newpoints, points, pointsLen, radians);
  return PathDistance(newpoints, T, pointsLen);
}
Point Centroid(Point *points, int pointsLen)
{
  Point p;
  p.X = 0.0;
  p.Y = 0.0;
  for (int i = 0; i < pointsLen; i++) {
    p.X += points[i].X;
    p.Y += points[i].Y;
  }
  p.X /= pointsLen;
  p.Y /= pointsLen;
  return p;
}
Rectangle BoundingBox(Point *points, int pointsLen) {
  float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
  for (int i = 0; i < pointsLen; i++) {
    minX = MIN(minX, points[i].X);
    minY = MIN(minY, points[i].Y);
    maxX = MAX(maxX, points[i].X);
    maxY = MAX(maxY, points[i].Y);
  }
  Rectangle r = { minX, minY, maxX - minX, maxY - minY };
  return r;
}
float PathDistance(Point *pts1, Point *pts2, int pointsLen)
{
  float d = 0.0;
  for (int i = 0; i < pointsLen; i++) // assumes pts1.length == pts2.length
    d += Distance(pts1[i], pts2[i]);
  return d / pointsLen;
}
float PathLength(Point *points, int pointsLen)
{
  float d = 0.0f;
  for (int i = 1; i < pointsLen; i++)
    d += Distance(points[i - 1], points[i]);
  return d;
}
float Distance(Point p1, Point p2)
{
  float dx = p2.X - p1.X;
  float dy = p2.Y - p1.Y;
  return sqrtf((dx*dx) + (dy*dy));
}
float Deg2Rad(float d) {
  return d * PI / 180.0f;
}


// =====================================================================================
Point touchHistory[NUMPOINTS];
uint8_t touchHistoryLen; // how many points in touchHistory
Point touchSum; // sum of points so far
bool touchLastPressed; // Is a finger on the screen? If not, next press we should start from scratch
uint8_t touchCnt; // how many touches are in touchSum
uint8_t touchDivisor; // how many touch points do we sum to get the next point?
int touchDistance; // distance dragged


/// initialise stroke detection
void unistroke_init() {
  touchHistoryLen = 0;
  touchDivisor = 1;
  touchLastPressed = false;
}

#ifdef LCD_WIDTH
/// Called when a touch event occurs, returns 'true' if an event should be created (by calling unistroke_getEventVar)
bool unistroke_touch(int x, int y, int dx, int dy, int pts) {
  if (!pts) { // touch released
    touchLastPressed = false;
    return touchDistance > LCD_WIDTH/2; // fire off an event IF the distance is large amount
  }
  // now finger is definitely pressed
  if (!touchLastPressed) {
    // reset everything if this is the first time
    touchHistoryLen = 0;
    touchDivisor = 1;
    touchSum.X = 0;
    touchSum.Y = 0;
    touchCnt = 0;
    touchDistance = 0;
    touchLastPressed = true;
  }
  // append to touch distance
  touchDistance += int_sqrt32((dx*dx)+(dy*dy));

  // store the sum of touch points
  touchSum.X += x;
  touchSum.Y += y;
  touchCnt++;
  // now add the point
  if (touchCnt >= touchDivisor) {
    if (touchHistoryLen==NUMPOINTS) {
      // if too many points, halve everything
      int j = 0;
      for (int i=0;i<NUMPOINTS;i+=2) {
        touchHistory[j].X = (touchHistory[i].X + touchHistory[i+1].X) / 2;
        touchHistory[j].Y = (touchHistory[i].Y + touchHistory[i+1].Y) / 2;
        j++;
      }
      touchHistoryLen = touchHistoryLen/2;
      touchDivisor += 2; // halve all future points too
    }
    // add in point
    touchHistory[touchHistoryLen].X = touchSum.X / touchCnt;
    touchHistory[touchHistoryLen].Y = touchSum.Y / touchCnt;
    touchHistoryLen++;
    touchSum.X = 0;
    touchSum.Y = 0;
    touchCnt = 0;
  }
  return false;
}
#endif


/// Convert an array containing XY values to a unistroke var
JsVar *unistroke_convert(JsVar *xy) {
  uint8_t points8[NUMPOINTS*2];
  unsigned int bytes = jsvIterateCallbackToBytes(xy, points8, sizeof(points8));
  int pointCount = bytes/2;
  Unistroke uni = newUnistroke8(points8, pointCount);
  return jsvNewStringOfLength(sizeof(uni), (char *)&uni);
}

/// recognise a single stroke
float unistroke_recognise_one(JsVar *strokeVar,  Unistroke *candidate) {
  float d = FLT_MAX;
  JSV_GET_AS_CHAR_ARRAY(strokePtr, strokeLen, strokeVar)
  if (strokePtr && strokeLen==sizeof(Unistroke)) {
    Unistroke *uni = (Unistroke*)strokePtr;
    /*if (useProtractor)
      d = OptimalCosineDistance(this.Unistrokes[i].Vector, candidate.Vector); // Protractor
    else*/
      d = DistanceAtBestAngle(candidate->points, NUMPOINTS, uni->points, -AngleRange, +AngleRange, AnglePrecision); // Golden Section Search (original $1)
  }
  jsvUnLock(strokeVar);
  return d;
}

/// Given an object containing values created with unistroke_convert, compare against a unistroke
JsVar *unistroke_recognise(JsVar *strokes,  Unistroke *candidate) {
  JsVar *u = 0;
  float b = FLT_MAX;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, strokes);
  while (jsvObjectIteratorHasValue(&it)) {  // for each unistroke template
    JsVar *strokeVar = jsvObjectIteratorGetValue(&it);
    float d = unistroke_recognise_one(strokeVar, candidate);
    if (d < b) {
      b = d; // best (least) distance
      jsvUnLock(u);
      u = jsvObjectIteratorGetKey(&it); // unistroke index
    }
    jsvUnLock(strokeVar);
    jsvObjectIteratorNext(&it);
  }
  // CERTAINTY = useProtractor ? (1.0 - b) : (1.0 - b / (0.5 * Diagonal))
  return u ? jsvAsStringAndUnLock(u) : 0;
}

/// Given an object containing values created with unistroke_convert, compare against an array containing XY values
JsVar *unistroke_recognise_xy(JsVar *strokes, JsVar *xy) {
  uint8_t points8[NUMPOINTS*2];
  unsigned int bytes = jsvIterateCallbackToBytes(xy, points8, sizeof(points8));
  int pointCount = bytes/2;
  Unistroke uni = newUnistroke8(points8, pointCount);
  return unistroke_recognise(strokes, &uni);
}

JsVar *unistroke_getEventVar() {
  if (!touchHistoryLen) return 0;
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  JsVar *a = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, touchHistoryLen*2);
  if (a) {
    JsVar *s = jsvGetArrayBufferBackingString(a,NULL);
    size_t n=0;
    for (int i=0;i<touchHistoryLen;i++) {
      jsvSetCharInString(s, n++, (char)touchHistory[i].X, false);
      jsvSetCharInString(s, n++, (char)touchHistory[i].Y, false);
    }
    jsvUnLock(s);
    jsvObjectSetChildAndUnLock(o, "xy", a);
  }
  JsVar *bangle = jsvObjectGetChild(execInfo.root, "Bangle", 0);
  if (bangle) {
    JsVar *strokes = jsvObjectGetChild(bangle, "strokes", 0);
    if (jsvIsObject(strokes)) {
      Unistroke uni = newUnistroke(touchHistory, touchHistoryLen);
      jsvObjectSetChildAndUnLock(o, "stroke", unistroke_recognise(strokes, &uni));
    }
    jsvUnLock2(bangle, strokes);
  }

  return o;
}


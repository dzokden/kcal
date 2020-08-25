// Julian day, delta T and other time routines
/*********************************************************************************
Licence for Kcal - Kalacakra calendar software

Copyright (c) 2011-2013 Edward Henning

Permission is hereby granted, free of charge, to any person  obtaining a copy of
this software and associated documentation files (the "Software"), to deal in the
Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
************************************************************************************/

#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include "jd.h"
#include "bcd.h"

double get_deltaT ( double );

extern signed char bcda[BCDMAX];
extern int d_d, m_m, y_y;

double jul_dat ( int, int, int );
double sgnd ( double ); // Returns the double sign of X
int gregd2jul ( int, int, int );

void chg_tim ( int, int, int, int, int );
int leapyr ( int );

int w_d, w_m, w_y;
extern int  d_d, m_m, y_y;
int ltm, lth;

// These are for intermediate calculations only:
int wd, wm, wy; // Western dates
int doweek;     // Day of the week
extern char *dayoweek[7]; // 0 = Saturday

// Find delta T, returns number of seconds
// Method from: http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html
double get_deltaT ( double JDate )
  {
    double y;
    double u, u2, u3, u4, u5, u6;
    double dT;
    double t, t2, t3, t4, t5, t6, t7;

    y = (double) y_y + ((double) ( m_m - 1 ) )/ 12.0 + ((double) (d_d - 1 ))/ 360.0;

    if ( y < -500.0 )
      {
        u = (y - 1820.0 )/100.0;
        dT = -20.0 + 32.0 * u * u;
      }
    else if ( y >= -500.0 && y < 500.0 )
      {
        u = y / 100.0;
        u2 = u * u;
        u3 = u2 * u;
        u4 = u2 * u2;
        u5 = u2 * u3;
        u6 = u3 * u3;
        dT = 10583.6 - 1014.41 * u + 33.78311 * u2 - 5.952053 * u3
              - 0.1798452 * u4 + 0.022174192 * u5 + 0.0090316521 * u6;
      }
    else if ( y >= 500.0 && y < 1600.0 )
      {
        u = ( y - 1000.0 ) / 100.0;
        u2 = u * u;
        u3 = u2 * u;
        u4 = u2 * u2;
        u5 = u2 * u3;
        u6 = u3 * u3;
        dT = 1574.2 - 556.01 * u + 71.23472 * u2 + 0.319781 * u3
             - 0.8503463 * u4 - 0.005050998 * u5 + 0.0083572073 * u6;
      }
    else if ( y >= 1600.0 && y < 1700.0 )
      {
        t = y - 1600.0;
        t2 = t * t;
        t3 = t * t2;
        dT = 120.0 - 0.9808 * t - 0.01532 * t2 + t3 / 7129.0;
      }
    else if ( y >= 1700.0 && y < 1800.0 )
      {
        t = y - 1700.0;
        t2 = t * t;
        t3 = t * t2;
        t4 = t2 * t2;        
        dT = 8.83 + 0.1603 * t - 0.0059285 * t2 + 0.00013336 * t3 - t4 / 1174000.0;
      }
    else if ( y >= 1800.0 && y < 1860.0 )
      {
        t = y - 1800.0;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t2 * t2;
        t5 = t2 * t3;
        t6 = t3 * t3;
        t7 = t3 * t4;
        dT = 13.72 - 0.332447 * t + 0.0068612 * t2 + 0.0041116 * t3 - 0.00037436
             * t4 + 0.0000121272 * t5 - 0.0000001699 * t6 + 0.000000000875 * t7;
      }
    else if ( y >= 1860.0 && y < 1900.0 )
      {
        t = y - 1860.0;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t2 * t2;
        t5 = t2 * t3;
        dT = 7.62 + 0.5737 * t - 0.251754 * t2 + 0.01680668 * t3
                - 0.0004473624 * t4 + t5 / 233174;
      }
    else if ( y >= 1900.0 && y < 1920.0 )
      {
        t = y - 1900.0;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t2 * t2;
        dT = -2.79 + 1.494119 * t - 0.0598939 * t2 + 0.0061966 * t3 - 0.000197 * t4;
      }
    else if ( y >= 1920.0 && y < 1941.0 )
      {
        t = y - 1920.0;
        t2 = t * t;
        t3 = t2 * t;
        dT = 21.20 + 0.84493 * t - 0.076100 * t2 + 0.0020936 * t3;
      }
    else if ( y >= 1941.0 && y < 1961.0 )
      {
        t = y - 1950.0;
        t2 = t * t;
        t3 = t2 * t;
        dT = 29.07 + 0.407 * t - t2 / 233.0 + t3 / 2547.0;
      }
    else if ( y >= 1961.0 && y < 1986.0 )
      {
        t = y - 1975.0;
        t2 = t * t;
        t3 = t2 * t;
        dT = 45.45 + 1.067 * t - t2 / 260.0 - t3 / 718.0;
      }
    else if ( y >= 1986.0 && y < 2005.0 )
      {
        t = y - 2000.0;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t2 * t2;
        t5 = t2 * t3;
        dT = 63.86 + 0.3345 * t - 0.060374 * t2 + 0.0017275 * t3 +
             0.000651814 * t4 + 0.00002373599 * t5;
      }
    else if ( y >= 2005.0 && y < 2050.0 )
      {
        t = y - 2000.0;
        t2 = t * t;
        dT = 62.92 + 0.32217 * t + 0.005589 * t2;
      }
    else if ( y >= 2050.0 && y < 2150.0 )
      {
        dT = -20.0 + 32.0 * ((y-1820.0)/100.0) * ((y-1820.0)/100.0)
             - 0.5628 * (2150.0 - y);
      }
    else if ( y >= 2150.0 )
      {
        u = ( y - 1820.0 ) / 100.0;
        dT = -20 + 32 * u * u;
      }
    return (dT);
  } // END - get_deltaT

// Test routine for checking Julian day calculations
void jul_chk ( void )
{
  int jd, d, m, y, julday2;
  int more = 1;
// These routines checked from JD 0 to year 10,000 CE. Not checked for
// individual date accuracy yet.
  jd = 0;
  do
    {
//  jul2date ( jd );
    if ( jd % 500 == 0 )
      printf ( "JD %ld = %ld/%ld/%ld\n", jd, wd, wm, wy );
    d = wd; m = wm; y = wy;
    wd = 0; wm = 0; wy = 0;
    julday2 = gregd2jul ( d, m, y );
    if ( jd != julday2 )
      {
        printf ( "ERROR: JD = %ld, JULDAY2 = %ld\n", jd, julday2 );
        getch ();
      }
    jd = jd + 1;
    } while ( more );
}

int gregd2jul ( int D, int M, int Y )
// Returns the Julian day number for the day.
// 1990 25th June = Julian day 2448068, at noon Julian date = 2448068.0.
// See Explanatory Supplement (1992), p. 600.
{
  int JD;
  int calndar;  // Gregorian = 1.

// In change from Julian to Gregorian calendars, in 1582, Oct 4th was followed
// by Oct 15th

  if ( Y > 1582 )
    calndar = 1;
  else if ( Y < 1582 )
    calndar = 0;
  else
    {
      if ( M > 10 )
        calndar = 1;
      else if ( M < 10 )
        calndar = 0;
      else
        {
          if ( D >= 15 )
            calndar = 1;
          else
            calndar = 0;
        }
    }
  if ( calndar )  // Gregorian:
    {
      // - the following is from ESAA, 1992, p. 604.
      JD = D + ( 1461 * ( Y + 4800 + ( M - 14 ) / 12 ) ) / 4
         + ( 367 * ( M - 2 - 12 * ( ( M - 14 ) / 12 ) ) ) / 12
         - ( 3 * ( ( Y + 4900 + ( M - 14 ) / 12 ) / 100 ) ) / 4 - 32075;
    }
  else  // Julian:
    {
      JD = 367 * Y - ( 7 * ( Y + 5001 + ( M - 9 ) / 7 )) / 4
           + ( 275 * M ) / 9 + D + 1729777;
    }
  return ( JD );
}

void jul2date ( int jd, struct tmst *te )
  {
    int l, n, j, k, i;

// This algorithm is from photcopied notes, from James Neely.
// Also, checked with ESAA, 1992, p. 604
// Calculates date, at noon on which the Julian date starts.
// Julian 0 starts Greenwich mean noon on 1st Jan 4713 BC, Julian proleptic
// calendar.
// In change from Julian to Gregorian calendars, in 1582, Oct 4th was followed
// by Oct 15th

// First, get day of week:

  doweek = jd - 7 * (( jd + 1 ) / 7 ) + 2;
  if ( doweek == 7 )
    doweek = 0;
  if ( doweek > 7 )
    {
      printf ( "ERROR IN DAY OF WEEK ROUTINE:\n" );
      getch ();
    }

  if ( jd >= 2299161 )  // Gregorian calendar:
    {
// This has been tested between March 1, 1600 and Jan 31, 2100
    l = jd + 68569;
    n = ( 4 * l ) / 146097;
    l = l - ( 146097 * n + 3 ) / 4;

//  wy = 4000L * ( l + 1L ) / 1461001L;
    l2bcd ( bcda, 4000 );
    mulbcdl ( bcda, bcda, l + 1 );
    divbcdl ( bcda, bcda, 1461001 );
    wy = bcd2l ( bcda );
    l = l - ( 1461 * wy ) / 4 + 31;
    wm = ( 80 * l ) / 2447;
    wd = l - ( 2447 * wm ) / 80;
    l = wm / 11;
    wm = wm + 2 - 12 * l;
    wy = 100 * ( n - 49 ) + wy + l;
// j = month, k = day, i = year
    }
  else // Julian calendar
    {
      j = jd + 1402;
      k = ( j - 1 ) / 1461;
      l = j - 1461 * k;
      n = ( l - 1 ) / 365 - l / 1461;
      i = l - 365 * n + 30;
      j = ( 80 * i ) / 2447;
      wd = i - ( 2447 * j ) / 80;
      i = j / 11;
      wm = j + 2 - 12 * i;
      wy = 4 * k + n + i - 4716;
    }

    te->yy = (int) wy;
    te->mm = (int) wm;
    te->dd = (int) wd;
    te->dow = (int) doweek;

  } // END - jul2date ( )

double jul_dat ( int d, int m, int y )
// Returns the x.5 value - at GMT zero hours at the beginning of the civil day.
// eg: 25 June 1990 = Julian day 2448068, at noon Julian date = 2448068.0.
// Original test code retained and commented out for illustration.
  {
    int JD; // D, M, Y;
//  d = 25; m = 6; y = 1990;

// This returns JD at zero hour UT, noon GMT:
    JD = gregd2jul ( d, m, y );

//    printf ( "First JD = %f\n", (double) JD - 0.5 );  // This worries me!
//    getch();

// We return the Julian date for zero hours GMT:
    return ( (double) JD - 0.5 );
  }

void chg_tim ( int yr, int mo, int dy, int hr, int mn )
  {
    if ( yr < 0 || mo < 0 || dy < 0 || hr < 0 || mn < 0 ) /** SUBTRACT **/
      {
        if ( mn )
          {
            ltm = ltm + mn;
            if ( ltm < 0 )
              {
                hr = -1;
                ltm = ltm + 60;
              }
          }
        if ( hr )
          lth = lth + hr;
        if ( lth < 0 )
          {
            dy = -1;
            lth = lth + 24;
          }
        if ( dy )
          d_d = d_d + dy;
        if ( d_d <= 0 )
          {
            mo = -1;
            if ( m_m == 5 || m_m == 7 || m_m == 10 || m_m == 12 )
              d_d += 30;
            else if ( m_m == 3 )
              {
                if ( leapyr ( y_y ) )
                  d_d += 29;
                else
                  d_d += 28;
              }
            else
              d_d += 31;
          }
        if ( mo )
          m_m = m_m + mo;
        if ( m_m <= 0 )
          {
            m_m = m_m + 12;
            yr = -1;
          }
        if ( yr )
          y_y = y_y + yr;
      }
    else                /*** ADD ***/
      {
        if ( mn )
          ltm = ltm + mn;
        if ( ltm >= 60 )
          {
            hr = 1;
            ltm = ltm - 60;
          }
        if ( hr )
          lth = lth + hr;
        if ( lth >= 24 )
          {
            dy = 1;
            lth = lth - 24;
          }
        if ( dy )
          d_d = d_d + dy;
        if ( d_d >= 29 && m_m == 2 )
          {
            if ( leapyr ( y_y ) )
              {
                if ( d_d >= 30 )
                  {
                    mo = 1;
                    d_d = d_d - 29;
                  }
              }
            else
              {
                mo = 1;
                d_d = d_d - 28;
              }
          }
        else if ( d_d >= 31 && ( m_m == 4 || m_m == 6 || m_m == 9 || m_m == 11
                  ) )
          {
            mo = 1;
            d_d = d_d - 30;
          }
        else if ( d_d >= 32 )
           {
             mo = 1;
             d_d = d_d - 31;
           }
        if ( mo )
          m_m = m_m + mo;
        if ( m_m > 12 )
          {
            m_m = m_m - 12;
            yr = 1;
          }
        if ( yr )
          y_y = y_y + yr;
      }
  }

int leapyr ( int y )
  {
    if ( y % 4 )
      return (0);
    else if ( !(y % 400) )
      return (1);
    else if ( !(y % 100) )
      return (0);
    return (1);
  }

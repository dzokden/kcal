/* KALACAKRA CALENDAR CALCULATOR, K1.C */

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
/* #include <conio.h> */
#include <curses.h>
#include <string.h>
/* #include <dos.h> */
#include <math.h>
#include "jd.h"
#include "kcal.def"

int get_tenbrel ( int, int );
void get_bishti_str ( int );
int chk_eclipse ( double, int );
double jul_dat ( int, int, int );
void sun_mon ( double );
double modfeh ( double, double );
void chg_tim ( int, int, int, int, int );
void prn_cal ( void );
int chk_spec ( int, int);
void get_new_moon ( double *, double *, double, double );
double get_tithi ( double, double );
int get_NFMoon ( double, int );
int chk_sign ( double, double );
double get_deltaT ( double );
double getmeanSun ( double );
double getmeanSunmeannewMoon ( double );
int getphrod ( int, int, double );
void conv2hms ( int*, int*, int*, int, int, int );

char filnam[14];
char ch_num[8];

char eclps_str[80];

int d_bug = 0;
char cal_site[50];
double site_long;
double jul_factor;
double jul_SC, jul_NFM; // Values for time of sign change and new/full Moon.

int  d_d, m_m, y_y; // Globals, because needed by get_deltaT ()
int h, m, s;
int  target_year;
int  first_Kmnth_flag, last_mnth_flag;
int juldat;
int yan_kwong_flg = 0;
int zin_phung_flg = 0;

char bishti_str[12];

struct new_moon_details
  {
    int dd;
    int mm;
    int yy;
    int Zsignstart;    // Range: 0-11
    int Zsignend;      // Range: 0-11
    int mnthnumK;      // Range: 1-12 - Kalacakra month number: 1 = nag zla
    int intercal;
    int nxtintercal;   // Flag for repeated month - the next is intercalary
    int delaymth;
// These new, may not be needed in the long term:
    double julf;  // Julian date of true new Moon    
    double meansol; // Mean Sun at true new Moon.    
  } newmon[20];   // Maximum index reached in testing = 17
  
int intercalfollows = 1; // Flag to indicate intercalary takes the qualities of the previous
	 		// normal month. For Tibetan style, set this to zero. NEW July 2013

extern struct eclipse_details
  {
    int partial;
    int annular;
    int total;
    float mag;
  } eclps;

extern double mol, sol;

double  juldatf; // Julian day
int   dayofweek;   // Day of the week

// Chinese data:
int c_genday;
int sd_clunmanx;
int ld_parx, sd_parx;
int ld_smex, sd_smex;
int ld_planx, ld_lunmx;
int ldo_animx, ldo_elemx, ldo_parx, ldo_smex;  // for omitted days.
int yr_gender;    // 1 = female, odd; 0 = male, even
int yr_animx, mt_animx, ld_animx, sd_animx;
int yr_elemx, mt_elemx, ld_elemx, sd_elemx;
int yr_smex;

FILE *fptgt;
FILE *fptgt_d; // For debug information
char outbuf[200];
char outbufSC[60];
char outbufNFM[50];

extern char  *e_str[];

int get_tenbrel ( int m, int t )
  { 
    int sx, td;
    sx = m - 10;
    if ( sx < 0 )
      sx = sx + 12;
    if ( t <= 15 )
      {
        if ( t <= 12 )
          td = sx + ( t - 1 );
        else
          td = sx + ( t - 11 );
      }
    else
      {
        if ( t <= 27 )
          td = sx - ( t - 16 );
        else
          td = sx - ( t - 2 );
      }  
    while ( td < 0 )
      td = td + 12;
    while ( td > 11 )
      td = td - 12;       
    return ( td );  
  } // END - get_tenbrel

void get_bishti_str ( int tt )
  {
    bishti_str[0] = 0;
    if ( tt == 4 )
      strcpy ( bishti_str, "bishti E." );
    else if ( tt == 8 )
      strcpy ( bishti_str, "bishti S." );
    else if ( tt == 11 )
      strcpy ( bishti_str, "bishti W." );
    else if ( tt == 15 )
      strcpy ( bishti_str, "bishti N." );
    else if ( tt == 18 )
      strcpy ( bishti_str, "bishti SE." );
    else if ( tt == 22 )
      strcpy ( bishti_str, "bishti SW." );
    else if ( tt == 25 )
      strcpy ( bishti_str, "bishti NW." );
    else if ( tt == 29 )
      strcpy ( bishti_str, "bishti NE." );
  } // END - get_bishti_str ()

void main (argc, argv)
    int argc;
    char *argv[];
  {
    int    finish, i, hit;
    double moon_elong, last_moon_elong;
    double NM_juldat, NM_sol, sol_hold;
    double NM_meansol; // Mean Sun at new Moon.
    int    mnth12_found, just_done_newmoon;
    int    first_new_year_found, second_new_year_found;
    int    intercal_found; // For testing only.

    if ( argc > 1 )
      {
        for ( i = 1; i < argc; ++i )
          {
            if ( i == 1 )  // file name
              strcpy ( filnam, argv[i] );
            else if ( i == 2 )  // site name
              strcpy ( cal_site, argv[i] );
            else if ( i == 3 )  // Year
              {
                strcpy ( ch_num, argv[i] );
                y_y = atoi ( ch_num );
                printf ( "Kalacakra calendar software. Version 3.1\n") ;
                printf ( "Target year = %d\n", y_y );
              }
            else if ( i == 4 )  // Longitude
              {
                strcpy ( ch_num, argv[i] );
                site_long = atof ( ch_num );
                printf ( "Longitude = %f\n", site_long );
              }
          } // END - for ( i = 1; i < argc; ++i )
        if ( cal_site[0] )
          {
            i = 0;
            do
              {
                if ( cal_site[i] == '_' )
                  cal_site[i] = 32;
                ++i;
              } while ( cal_site[i] > '\0' );
          }
      } // END - if ( argc > 1 )
    else
      {
        site_long = 0.0; // For Greenwich as default
        printf ("\n\nEnter the year required: ");
        scanf ("%d", &y_y );
        printf ("\n");
      }
    target_year = y_y;
    mnth12_found = 0;
    first_new_year_found = 0;
    second_new_year_found = 0;
    intercal_found = 0;  // FOR TESTING ONLY
    for ( i = 0; i < 20; ++i ) // Clear these, just to be safe.
      {
        newmon[i].intercal = 0;
        newmon[i].delaymth = 0;
        newmon[i].nxtintercal = 0;
      }

// Kalacakra New Year cannot start before about 22nd Feb. Start searching from 1st Jan:

    d_d = 1;
    m_m = 1;
    last_moon_elong = 0.0;
    hit = 0;
    finish = 0;
    just_done_newmoon = 0;
    jul_factor = 5.0/24.0 - site_long / 360.0;  // for 5am LMST at site

// First, go through year finding new moons and setting month definitions.
    while ( finish == 0 )
      {
        juldatf = jul_dat ( d_d, m_m, y_y ); // returns the x.5 value.
                                             // ie. beginning of UT day.
        juldatf = juldatf + jul_factor; // Add time zone factor.
// juldatf is now equivalent to 5am LMST on our target meridian
        sun_mon ( juldatf ); // This adds Delta T
        moon_elong = mol - sol;
        sol_hold = sol;  // Solar longitude at 5am LMST.
        if ( moon_elong < 0.0 )
          moon_elong = moon_elong + 360.0;
        if ( moon_elong < last_moon_elong ) // We have gone past a New Moon
          {
            newmon[hit].dd = d_d;  // First day after new Moon.
            newmon[hit].mm = m_m;
            newmon[hit].yy = y_y;

// We need to find exact time of New Moon:
            get_new_moon ( &NM_juldat, &NM_sol, juldatf, moon_elong );
            just_done_newmoon = 1;

// We store an index number for the solar zodiac sign at new Moon.

//-----------------------------------------------------------------------

// For mean Sun at mean new Moon: 

// Use these four lines if month definition is mean Sun at mean new Moon:
// Should we calculate exact time of mean new moon - perhaps useful??
// fptgt3 = fopen ( "tst_msmm.txt", "a" ); // Mean Sun, true new Moon
            NM_meansol = getmeanSunmeannewMoon ( NM_juldat ); // This adds Delta T
            newmon[hit].julf = NM_juldat;
            newmon[hit].meansol = NM_meansol;
// This is the line actually to use:            
            newmon[hit].Zsignstart = (int) floor ( NM_meansol / 30.0 );

//-----------------------------------------------------------------------

// Use this line if month definition is true Sun at true new Moon:
//          newmon[hit].Zsignstart = (int) floor ( NM_sol / 30.0 );

//-----------------------------------------------------------------------

// Use these two lines if month definition is mean Sun at true new Moon:
//            NM_meansol = getmeanSun ( NM_juldat ); // This adds Delta T
//            newmon[hit].Zsignstart = (int) floor ( NM_meansol / 30.0 );

// Show the user we are actually doing something:
            printf ( "Jul: %f, Mean Sun = %f, True Sun = %f\n",
                      NM_juldat, NM_meansol, NM_sol );

// Use this line if month definition is daybreak, true Sun:
//          newmon[hit].Zsignstart = (int) floor ( sol_hold / 30.0 );

            if ( hit > 0 ) // Not our first new Moon
              {
                newmon[hit-1].Zsignend = newmon[hit].Zsignstart;
// Do we have a month in which the Sun does not change sign?
                if ( newmon[hit-1].Zsignstart == newmon[hit-1].Zsignend )
                  {
                    // Does intercalary take the month before or delay?
                    if ( intercalfollows == 0 ) // Tibetan style
                      newmon[hit-1].mnthnumK = newmon[hit-1].Zsignend + 2;
                    else  // Kalacakra style
                      newmon[hit-1].mnthnumK = newmon[hit-1].Zsignend + 1; //CHECK !!!????
                    if ( newmon[hit-1].mnthnumK > 12 )
                      newmon[hit-1].mnthnumK -= 12;
                    newmon[hit-1].intercal = 1;
                    newmon[hit].delaymth = 1;                    
                    if ( hit > 1 ) // Flag for repeated month - the next is intercalary
                      newmon[hit-2].nxtintercal = 1;
                    printf ( "Intercalary month: %d\n", newmon[hit-1].mnthnumK );
                    if ( hit < 3 && newmon[hit-1].mnthnumK == 12 ) // FOR
                                                               // TESTING
                      mnth12_found = 1;
                    if ( intercal_found )  // TESTING ONLY
                      printf ( "TWO INTERCALARY MONTHS!!!\n" );
                    intercal_found = 1;
                  }
                else  // Normal month
                  {
                    newmon[hit-1].mnthnumK = newmon[hit-1].Zsignend + 1;
                    if ( newmon[hit-1].mnthnumK > 12 )
                      newmon[hit-1].mnthnumK -= 12;
                    newmon[hit-1].intercal = 0;

// Following two "if"s are for checking for possible problems:
                    if ( hit < 3 && newmon[hit-1].mnthnumK == 12 )
                      mnth12_found = 1;
                    if ( newmon[hit-1].Zsignend - newmon[hit-1].Zsignstart == 2
                                                                           )
                      {
                        printf ( "DOUBLE SIGN ENTRY: %d\n",
                                 newmon[hit-1].Zsignstart + 1 );
                        getch ();
                      }
                  }
              }
            if ( first_new_year_found == 0 )
              {
                if ( newmon[hit-1].mnthnumK == 1 )
                  {
                    first_Kmnth_flag = hit - 1;
                    first_new_year_found = 1;
                  }
// This to fix the missing of intercalary first months: 11/3/2008
// Point is: if we have already found the first month, do not mark a
// delayed month 1 as the first. Is there a similar problem with line above???
//                else if ( newmon[hit-1].mnthnumK == 12 && hit > 1
//                          && newmon[hit-2].mnthnumK != 12 )
//                  {
//                    first_Cmnth_flag = hit - 1;
//                  first_new_year_found = 1;
//                  }
              }
            if ( second_new_year_found == 0 && y_y > target_year &&
                 newmon[hit-1].mnthnumK == 1 )
              {
                last_mnth_flag = hit - 2;
                second_new_year_found = 1;
                finish = 1;
              }
            ++hit;
          }
// Do another day:
        last_moon_elong = moon_elong;

        if ( just_done_newmoon == 1 )
          {
            chg_tim ( 0, 0, 26, 0, 0 );
            just_done_newmoon = 0;
          }
        else
          chg_tim ( 0, 0, 1, 0, 0 );
      } // End of loop searching for new Moons

    if ( !mnth12_found )  // Keep this as an error check.
      {
        printf ( "Twelfth month not found\n" );
        getch ();
      }

    finish = 0;
    prn_cal ();
  } // END - main

// Check for special festival days, etc.
int chk_spec ( int m, int t )
  {
    switch ( m )
      {
        case 1:
          if ( t == 15L )
            {
              sprintf ( outbuf, "%s", "Revelation of the Kalacakra Tantra." );
              return (1);
            }
        break;
        case 2:
          if ( t == 8L )
            {
              sprintf ( outbuf, "%s", "Birth of the Buddha." );
              return (1);
            }
          else if ( t == 15L )
            {
              sprintf ( outbuf, "%s",
                "Enlightenment and Parinirvana of the Buddha." );
              return (1);
            }
        break;
        case 3:
          ;
        break;
        case 4:
          if ( t == 4L )
            {
              sprintf ( outbuf, "%s", "Turning of the Wheel of the Dharma." );
              return (1);
            }
          else if ( t == 15L )
            {
              sprintf ( outbuf, "%s",
                        "The Buddha's entry into the womb of his mother." );
              return (1);
            }
        break;
        case 5:
          ;
        break;
        case 6:
          ;
        break;        
        case 7:
          if ( t == 22L )
            {
              sprintf ( outbuf, "%s",
                "Descent of the Buddha from the realm of the gods." );
              return (1);
            }
        break;
        case 8:
          ;
        break;
        case 9:
          ;
          break;
        case 10:
          ;
        break;
        case 11:
          if ( t == 1L )
            {
              sprintf ( outbuf, "%s",
                "From 1st to 15th, Demonstration of Miracles." );
              return (1);
            }
        break;
        case 12:
          ;
        break;        
        default:
          ;
        break;
      }
    return (0);
  } // END - chk_spec ()

void get_new_moon ( double * NM_jdat, double * NM_sol, double juldat,
                    double melong )
  {
    int done = 0;
    do
      {
        juldat = juldat - ( melong / 13.2 ); // Moon moves 13.2 degrees a day
        sun_mon ( juldat ); // This adds Delta T
        melong = mol - sol;
        if ( melong > -0.00001 && melong < 0.00001 )
          done = 1;
        if ( melong < -330.0 )
          melong = melong + 360.0;
        else if ( melong > 330.0 )
          melong = melong - 360.0;
        if ( melong < -15.0 || melong > 15.0 )
          {
            printf ( "ERROR - Sun: %f, Moon: %f\n", sol, mol );
            getch ();
          }
      } while ( !done );
    *NM_jdat = juldat;
    *NM_sol = sol;
  } // END - get_new_moon

double get_tithi ( double tgtelong, double juldat )
// Get time of lunar day change
  {
    int done = 0;
    double el_diff, jul_nxt, melong;
    jul_nxt = juldat;
    do
      {
        sun_mon ( jul_nxt ); // This adds Delta T
        melong = mol - sol;
        if ( melong < 0.0 )
          melong = melong + 360.0;
        el_diff = tgtelong - melong;
        if ( el_diff > 180.0 )
          el_diff = 360.0 - el_diff;
        else if ( el_diff < -180.0 )
          el_diff = 360.0 + el_diff;
        if ( el_diff > -0.00001 && el_diff < 0.00001 )
          done = 1;
        else
          jul_nxt = jul_nxt + ( el_diff / 13.2 ); // Moon moves 13.2 degrees a day
        if ( el_diff > 15.0 || el_diff < -15.0 )
          {
            printf ( "PROBLEM\n" );
            getch ();
          }
      } while ( !done );
    return ( jul_nxt );
  } // END - get_tithi

int chk_sign ( double juldat, double sl )
// juldat is set to juldatf = time at daybreak at target location.
// Checking for change of Sun sign.
  {
    double jd, tgt, dif;
    int s1, s2, n, p, b;
    int done = 0;

    if ( sl > 360.0 ) // Old debug code
      {
        d_bug = 1;
        printf ( "Debug turned on...\n" );
        printf ( "sl = %f\n", sl );                
        fptgt_d = fopen ( "debug.dat", "w" );
        if ( fptgt_d == NULL )
          {
            printf ( "Error opening target file!\n" );
            getch ();
            return (0);
          }                
      }            

    s1 = (int) floor ( sl / 30.0 );
    jd = juldat + 1.0;
    sun_mon ( jd ); // This adds Delta T
    s2 = ( int ) floor ( sol / 30.0 );
   
    if ( s1 != s2 )
      {
        d_bug = 0;
        jd = juldat;
        tgt = (double) s2 * 30.0;
        if ( s2 == 0 )
          tgt = 360.0;
        dif = tgt - sl;
        do
          {
            if ( d_bug ) 
              printf ( "dif = %f\n", dif );
            jd = jd + dif; // Sun moves just less than 1 degree a day
            sun_mon ( jd );
            dif = tgt - sol;
            if ( dif > 180.0 )
              dif = dif - 360.0;                          
            if ( dif > -0.000001 && dif < 0.000001 ) // Much less than a second.
              done = 1;
          } while ( !done );

        jul_SC = jd;
        dif = 60.0 * ( jd - juldat );  // Seems right.
        n = (int) floor ( dif );
        dif = 60.0 * ( dif - (double) n );
        p = (int) floor ( dif );
        dif = 6.0 * ( dif - (double) p );
        b = (int) floor ( dif );
        conv2hms ( &h, &m, &s, n, p, b );
        sprintf ( outbufSC, "\x20\x20Sun enters %s at %d:%d:%d (%dh %dm %ds).",
                          ZsignsE[s2], n, p, b, h, m, s );
        d_bug = 0;
        return (1);
      }
    return (0);
  } // END - chk_sign ()

void prn_cal ( void )
  {
    int i, more, start_month;
    int mth_ind, pb;
    int tt, tt2, last_tt, tt_lhag, tt_chad, tt_chad_keep;
    int first_day;    
    int eclipse_flg;
    int  lunmanx, lunmannext, sbyorba, byedpa, byedpax;
    int NFM_flag, SC_flag, lmchg;
    double x;
    double moon_elong, sol1, mol1, moon_elong1;
    double sol2, mol2, moon_elong2;
    double tt1chg, ott1chg, tt2chg, tt1tgt, tt2tgt;
    double tt15jul, tt30jul;    
    double lmchgjdatf, tgtmol, dif;
    int lmcn, lmcp, lmcb; // Lunar mansion change time details.
    int phrod1, phrod2;
    int tenbrelx;

    last_tt = 30;   // As if we had just done a month, for beginning of year.
    tt_lhag = 0;
    tt_chad = 0;    
    tt_chad_keep = 0;    

    if ( filnam[0] == '\0' )
      sprintf ( filnam, "%d.dat", target_year );
    fptgt = fopen ( filnam, "w" );
    if ( fptgt == NULL )
      {
        printf ( "Error opening target file!\n" );
        getch ();
        return;
      }
     pb = target_year - 127; // Index for 60-year cycle
     pb = pb % 60;

     if ( cal_site[0] > 0 )
       {
         sprintf ( outbuf,
               "Calendar for %s, and other places near longitude %.2f",
                cal_site, site_long );
         fprintf ( fptgt, "%s\n\n", outbuf );
       }

// New year information
    sprintf ( outbuf, "Kalacakra New Year: %s/%s, %d.", PrabhavaS[pb], PrabhavaT[pb],
                            target_year );
    fprintf ( fptgt, "%s\n", outbuf );

    start_month = first_Kmnth_flag;

// Start of month loop *******************************
    for ( mth_ind = start_month; mth_ind <= last_mnth_flag; ++mth_ind )
      {

// Start printing month information:
        if ( newmon[mth_ind].intercal == 1 ) // Intercalary
          {
            sprintf ( outbuf, "Intercalary Month: %d, %s/%s, %s/%s.",
                  newmon[mth_ind].mnthnumK,
                  MnthS[ newmon[mth_ind].mnthnumK - 1 ], MnthT[ newmon[mth_ind].mnthnumK - 1 ],
                  M_seasonE[ newmon[mth_ind].mnthnumK - 1 ], M_seasonT[ newmon[mth_ind].mnthnumK - 1 ] );
            printf ( "%s\n", outbuf );                  
            fprintf ( fptgt, "\n%s\n\n", outbuf );
          }
        else if ( newmon[mth_ind].nxtintercal == 1 && intercalfollows == 1 ) // Repeated month
          { // Kalacakra style
            sprintf ( outbuf, "Repeated Month: %d, %s/%s, %s/%s, %s/%s.",
                      newmon[mth_ind].mnthnumK,
                      MnthS[ newmon[mth_ind].mnthnumK - 1 ],
                      MnthT[ newmon[mth_ind].mnthnumK - 1 ],
                      M_seasonE[ newmon[mth_ind].mnthnumK - 1 ], 
                      M_seasonT[ newmon[mth_ind].mnthnumK - 1 ],
                      ZsignsE[ newmon[ mth_ind ].Zsignend ],
                      ZsignsT[ newmon[ mth_ind ].Zsignend ] );
            printf ( "%s\n", outbuf );                                        
            fprintf ( fptgt, "\n%s\n\n", outbuf );
          }
        else if ( newmon[mth_ind].delaymth == 1 && intercalfollows == 0 ) // Delayed month
          { // Tibetan style
            sprintf ( outbuf, "Delayed Month: %d, %s/%s, %s/%s, %s/%s.",
                      newmon[mth_ind].mnthnumK,
                      MnthS[ newmon[mth_ind].mnthnumK - 1 ],
                      MnthT[ newmon[mth_ind].mnthnumK - 1 ],
                      M_seasonE[ newmon[mth_ind].mnthnumK - 1 ], 
                      M_seasonT[ newmon[mth_ind].mnthnumK - 1 ],
                      ZsignsE[ newmon[ mth_ind ].Zsignend ],
                      ZsignsT[ newmon[ mth_ind ].Zsignend ] );
            printf ( "%s\n", outbuf );                                        
            fprintf ( fptgt, "\n%s\n\n", outbuf );
          }          
        else // Not intercalary
          {
            sprintf ( outbuf, "Month: %d, %s/%s, %s/%s, %s/%s.",
                      newmon[mth_ind].mnthnumK,
                      MnthS[ newmon[mth_ind].mnthnumK - 1 ],
                      MnthT[ newmon[mth_ind].mnthnumK - 1 ],
                      M_seasonE[ newmon[mth_ind].mnthnumK - 1 ], 
                      M_seasonT[ newmon[mth_ind].mnthnumK - 1 ],
                      ZsignsE[ newmon[ mth_ind ].Zsignend ],
                      ZsignsT[ newmon[ mth_ind ].Zsignend ] );
            printf ( "%s\n", outbuf );                                        
            fprintf ( fptgt, "\n%s\n\n", outbuf );
          }          

// End printing month information.
// Set date for the beginning of this new month:

        d_d = newmon[mth_ind].dd;
        m_m = newmon[mth_ind].mm;
        y_y = newmon[mth_ind].yy;
        more = 1;
        first_day = 1; // Used as check for omitted first lunar day.

// Now go through each day at a time:

        do
          { // Start of day loop
            // for each solar day, calculate the tithi(s)/lunar day.
            NFM_flag = 0; // Flag for new or full Moon days.
            SC_flag = 0; // Flag for Sun sign change.
            juldatf = jul_dat ( d_d, m_m, y_y );
            eclipse_flg = 0;
            juldat = gregd2jul ( d_d, m_m, y_y );

// Code here to work out day of week.
// Need to be careful of different time zones.

            dayofweek = floor ( juldatf );

            if ( d_bug )
              {
                sprintf ( outbuf, "\n\nDate: %d / %d / %d\n", d_d, m_m, y_y );
                fprintf ( fptgt_d, "%s\n", outbuf );
                sprintf ( outbuf, "Juldat = %ld\n", juldat );
                fprintf ( fptgt_d, "%s\n", outbuf );
              }
            dayofweek = juldat + 2L;
            dayofweek = dayofweek % 7L;

            if ( d_bug )
              {
                sprintf ( outbuf, "Julian UT 0: %f\n", juldatf );
                fprintf ( fptgt_d, "%s\n", outbuf );
                sprintf ( outbuf, "Time zone factor = %f\n", jul_factor );
                fprintf ( fptgt_d, "%s\n", outbuf );
              }

            juldatf = juldatf + jul_factor; // Add time zone factor.

            if ( d_bug )
              {
                sprintf ( outbuf, "Target time: %f\n", juldatf );
                fprintf ( fptgt_d, "%s\n", outbuf );
              }

// Calculate longitudes of Sun and Moon at daybreak
            sun_mon ( juldatf ); // This adds Delta T
            moon_elong = mol - sol;

            if ( d_bug )
              {
                sprintf ( outbuf, "Solar longitude: %f\n", sol );
                fprintf ( fptgt_d, "%s\n", outbuf );
                sprintf ( outbuf, "Lunar longitude: %f\n", mol );
                fprintf ( fptgt_d, "%s\n", outbuf );
              }

            if ( moon_elong < 0.0 )
              moon_elong = moon_elong + 360.0;
            tt = 1 + (int) floor ( moon_elong / 12.0 );
            
            if ( tt == 2 && first_day )
              tt_chad_keep = 1;
            
            if ( sol > 360.0 ) // Has this ever happened? Maybe remove !!!
              {
                d_bug = 1;
                printf ( "Debug turned on...\n" );
                printf ( "sol = %f\n", sol );                
                fptgt_d = fopen ( "debug.dat", "w" );
                if ( fptgt_d == NULL )
                  {
                    printf ( "Error opening target file!\n" );
                    getch ();
                    return;
                  }                
              }            

            if ( d_bug )
              {
                sprintf ( outbuf, "Moon elongation: %f", moon_elong );
                fprintf ( fptgt_d, "%s\n", outbuf );
                printf ( "%s\n", outbuf );                
                sprintf ( outbuf, "Lunar date: %d", tt );
                fprintf ( fptgt_d, "%s\n", outbuf );
                printf ( "%s\n", outbuf );        
              }

// Check next lunar day, for omitted or duplicated:

            mol1 = mol; sol1 = sol; moon_elong1 = moon_elong;
            sun_mon ( juldatf + 1.0 ); // This adds Delta T
            mol2 = mol;
            sol2 = sol;
            moon_elong = mol2 - sol2;
            if ( moon_elong < 0.0 )
              moon_elong = moon_elong + 360.0;
            moon_elong2 = moon_elong;
            tt2 = 1 + floor ( moon_elong / 12.0 );
            if ( tt == tt2 )
              tt_lhag = 1;
            else
              {
                if ( tt2 < tt )
                  tt2 += 30;
                if ( tt2 - tt == 2 )
                  {
                    tt_chad = tt + 1;
                    if ( tt_chad > 30 )
                      tt_chad -= 30;
                  }
                else
                  tt_chad = 0;
              }

// Calculate here the time for tithi to change - if it does.
// This is all really only for test code, but may possibly
// be useful at some later point. Data in terms of Julian date

          if ( !tt_lhag ) // Then we have a change of tithi during the solar day
            {
              if ( tt_chad_keep && newmon[mth_ind].mnthnumK == 1 ) // Special case  
                { // This not really needed. Put here for pos. future use
                  tt2tgt = 12.0; // 12.0;
                  tt2chg = get_tithi ( tt2tgt, juldatf - 1.0 );
                }                                            
              if ( !tt_chad ) // Normal day, only one change to find
                {
                  // Find the elongations needed:
                  tt1tgt = (double) tt * 12.0;
                  if ( tt == 30L )
                    tt1tgt = 0.0;
// Search from juldatf, the time of current daybreak
                  tt1chg = get_tithi ( tt1tgt, juldatf );
//                degs2lms ( &nyidag, sol ); 
                  if ( tt == 15 ) // 4 lines new from tcal
                    tt15jul = tt1chg;  // These and others for tt15/30 not needed yet
                  else if ( tt == 30 )
                    tt30jul = tt1chg;                        
                }
              else // Two to find, omitted lunar date
                {
                  // Estimate the time of change for first tithi change:
                  tt1tgt = (double) tt * 12.0;
                  if ( tt == 30L )
                    tt1tgt = 0.0;
                  ott1chg = get_tithi ( tt1tgt, juldatf );
                  tt1chg = ott1chg; // Needed twice for test code.
                  
                  if ( tt == 15 )
                    tt15jul = tt1chg;
                  else if ( tt == 30 )
                    tt30jul = tt1chg;                   
                  
                  // Now the next:
                  tt2tgt = (double) ( tt2 - 1L ) * 12.0;
                  if ( tt2 - 1L == 30L )
                    tt2tgt = 0.0;
                  else if ( tt2 - 1L == 31L )
                    tt2tgt = 12.0;
                  tt2chg = get_tithi ( tt2tgt, juldatf );
                  if ( tt == 15 )
                    tt15jul = tt2chg;
                  else if ( tt == 30 )
                    tt30jul = tt2chg;                                     
                }
            }
// Second of duplicated, tithi will change:
          else if ( tt_lhag == 2 )
            {
              // Find the elongation needed:
              tt1tgt = (double) tt * 12.0;
              if ( tt == 30L )
                tt1tgt = 0.0;
              tt1chg = get_tithi ( tt1tgt, juldatf );
              if ( tt == 15 )
                tt15jul = tt1chg;
              else if ( tt == 30 )
                tt30jul = tt1chg;                                               
            }

// Put back the data:
          mol = mol1; sol = sol1; moon_elong = moon_elong1;

// Calculate lunar mansion at daybreak:

          lunmanx = (int) floor ( mol * 0.075 ); // 3/40;

          if ( d_bug )
            {
              sprintf ( outbuf, "Lunar mansion: %d\n", lunmanx );
              fprintf ( fptgt_d, "%s\n", outbuf );
            }

// Calculate time of lunar mansion change:

// In tcal, this is all done with:
//        lunmannext = mak_lmchange_string ( juldatf, doweek, lunmanx ); 
          mol1 = mol; sol1 = sol;
          tgtmol = (double) (lunmanx + 1) * 13.33333333; // 40/3
          if ( lunmanx == 26 )
            {
              tgtmol = 360.0;
              lunmannext = 0;
            }
          else
            lunmannext = lunmanx + 1;
          lmchgjdatf = juldatf;

          do {
               lmchgjdatf = lmchgjdatf + ( tgtmol - mol ) / 13.2;
               sun_mon ( lmchgjdatf );
               if ( mol < 6.0 )
                 mol = mol + 360.0;
             } while ( tgtmol - mol > 0.000001 || mol - tgtmol > 0.000001 );

          if ( lmchgjdatf >= juldatf + 1.0 )
            lmchg = 0;
          else
            {
              lmchg = 1;
              dif = 60.0 * ( lmchgjdatf - juldatf ); // Seems right.
              lmcn = (int) floor ( dif );
              dif = 60.0 * ( dif - (double) lmcn );
              lmcp = (int) floor ( dif );
              dif = 6.0 * ( dif - (double) lmcp );
              lmcb = (int) floor ( dif );
            }

// Put back the data:
          mol = mol1; sol = sol1;

// Calculate yoga:

          x = mol + sol;
          while ( x >= 360.0 )
            x = x - 360.0;          
          sbyorba = (int) floor ( ( x * 3.0 ) / 40.0 ); 

// Calculate karana:

          byedpa = (int) floor ( moon_elong / 6.0 );

          if ( byedpa == 0 )
            byedpax = 7;
          else if ( byedpa == 57 )
            byedpax = 8;
          else if ( byedpa == 58 )
            byedpax = 9;
          else if ( byedpa == 59 )
            byedpax = 10;
          else
            byedpax = ( byedpa - 1 ) % 7;

// Check for eclipses:

            if ( tt_lhag == 0 && tt_chad != 15 && tt == 15 ) // Normal day
              eclipse_flg = chk_eclipse ( juldatf, 15 );
            else if ( tt_lhag == 2 && tt == 15 ) // Duplicated day
              eclipse_flg = chk_eclipse ( juldatf, 15 );
            else if ( tt_chad == 15 ) // Omitted day
              eclipse_flg = chk_eclipse ( juldatf, 15 );
            else if ( tt_lhag == 0 && tt_chad != 30 && tt == 30 ) // Normal day
              eclipse_flg = chk_eclipse ( juldatf, 30 );
            else if ( tt_lhag == 2 && tt == 30 ) // Duplicated day
              eclipse_flg = chk_eclipse ( juldatf, 30 );
            else if ( tt_chad == 30 ) // Omitted day
              eclipse_flg = chk_eclipse ( juldatf, 30 );
              
// Now, start printing daily information, first,
// dealing with omitted lunar days:

            if ( tt_chad_keep )  // OMITTED TITHI, Lunar day 1
              { 
// LINE 1:
                sprintf ( outbuf, "1. Omitted lunar day." );                      
                fprintf ( fptgt, "%s\n", outbuf );
                if ( d_bug) fprintf ( fptgt_d, "%s\n", outbuf );                                
// LINE 2: (if special)
                if ( !newmon[mth_ind].delaymth &&
                  chk_spec ( newmon[mth_ind].mnthnumK, 1 ) )
                  fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbuf );
                         tt_chad_keep = 0;    
              } // END - if (  tt_chad_keep )  // OMITTED TITHI 1
              
            if ( tt_lhag == 0 || tt_lhag == 1 ) // NORMAL OR FIRST OF DUPLICATED
              {
// LINE 1:  NORMAL OR FIRST OF DUPLICATED DAY 

            if ( lmchg ) // If lunar mansion changes:
              {
            conv2hms ( &h, &m, &s, lmcn, lmcp, lmcb );              
            sprintf ( outbuf, "%d. %s, %d %s %d. %s/%s, %d:%d:%d (%dh %dm %ds): %s/%s; %s/%s, %s/%s.",
            tt, dayoweek[ (int) dayofweek ], d_d, wmonths[m_m-1],
            y_y, lunmanS[lunmanx], lunmanT[lunmanx], lmcn, lmcp, lmcb, h, m, s,
            lunmanS[lunmannext], lunmanT[lunmannext], yogaS[sbyorba], yogaT[sbyorba],
                      byedS[byedpax], byedT[byedpax] );
              }
            else
              {
            sprintf ( outbuf, "%d. %s, %d %s %d. %s/%s; %s/%s, %s/%s.",
                      tt, dayoweek[ (int) dayofweek ], d_d, wmonths[m_m-1],
                       y_y, lunmanS[lunmanx], lunmanT[lunmanx], yogaS[sbyorba], yogaT[sbyorba],
                      byedS[byedpax], byedT[byedpax] );
              }
            fprintf ( fptgt, "%s\n", outbuf );

// LINE 1B:  NORMAL OR FIRST OF DUPLICATED DAY 
            if ( lmchg ) // If lunar mansion changes:
              {
                phrod1 = getphrod ((int) dayofweek, lunmanx, mol1 );
                phrod2 = getphrod ((int) dayofweek, lunmannext, mol1 );
                sprintf ( outbuf, "%s-%s -> %s-%s, %s -> %s.",
                          wdayelem[(int) dayofweek], lmanelem[lunmanx],
                          wdayelem[(int) dayofweek], lmanelem[lunmannext],
                          phrodE[phrod1], phrodE[phrod2] );
              }
            else
              {
                phrod1 = getphrod ((int) dayofweek, lunmanx, mol1 );
                sprintf ( outbuf, "%s-%s, %s.",
                          wdayelem[(int) dayofweek], lmanelem[lunmanx],
                          phrodE[phrod1] );
              }
            fprintf ( fptgt, "\x20\x20%s\n", outbuf );

// LINE 1c: -- Debug code, not normally in use:

            tenbrelx = get_tenbrel ( newmon[mth_ind].mnthnumK, tt );                   
            get_bishti_str ( tt );             
            
// LINE 2:  NORMAL OR FIRST OF DUPLICATED DAY 
            if ( !bishti_str[0] )
              sprintf ( outbuf, "%s/%s", tenbrelT[tenbrelx], tenbrelE[tenbrelx] );
            else
              sprintf ( outbuf, "%s/%s, %s", tenbrelT[tenbrelx], tenbrelE[tenbrelx], bishti_str );
            fprintf ( fptgt, "\x20\x20%s\n", outbuf );
            
// Extra lines:
            if ( ( ( tt == 14 && tt_chad == 15 ) ||
                ( tt == 29 && tt_chad == 30 ) ) && eclipse_flg && tt_lhag == 0 )
              fprintf ( fptgt, "\x20\x20\x20\x20%s\n", eclps_str );

// This is skipped if repeated lunar day:

            if ( !newmon[mth_ind].intercal &&
                 chk_spec ( newmon[mth_ind].mnthnumK, tt ) )
              fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbuf );

            if ( chk_sign ( juldatf, sol ) )  // Check for change of sign
              // Result is put into outbufSC ready for printing.
              SC_flag = 1; // Flag for Sun sign change.

// This only works if 15 or 30 are not omitted days. This should be fixed:
            if ( ( tt == 15 || tt == 30 ) && tt_lhag != 1 )
              {
                NFM_flag = 1; // Flag for new or full Moon days.
                if ( !get_NFMoon ( juldatf, tt ) ) // Error code:
                  {
                    printf ( "Real problem - NO New/Full Moon found\n" );
                    getch ();
                  }
              }

            if ( SC_flag && NFM_flag )
              {
                if ( jul_SC < jul_NFM ) // Sign change goes first:
                  {
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                  }
                else // New/Full Moon goes first.
                  {
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                  }
              }
            else if ( SC_flag || NFM_flag ) // If only one needs to be printed.
              {
                if ( SC_flag )
                  {
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                  }
                else if ( NFM_flag )
                  {
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                  }
              }

                if ( NFM_flag && tt_lhag != 1 && eclipse_flg )
                  fprintf ( fptgt, "\x20\x20\x20\x20%s\n", eclps_str );
              } // END - if ( tt_lhag == 0 || tt_lhag == 1 )         
            else if ( tt_lhag == 2 ) // SECOND OF DUPLICATED 
              {
                
// LINE 1: SECOND OF DUPLICATED
            if ( lmchg ) // If lunar mansion changes:
              {
            conv2hms ( &h, &m, &s, lmcn, lmcp, lmcb );              
            sprintf ( outbuf, "%d. %s, %d %s %d. %s/%s, %d:%d:%d (%dh %dm %ds): %s/%s; %s/%s, %s/%s.",
            tt, dayoweek[ (int) dayofweek ], d_d, wmonths[m_m-1],
            y_y, lunmanS[lunmanx], lunmanT[lunmanx], lmcn, lmcp, lmcb, h, m, s,
            lunmanS[lunmannext], lunmanT[lunmannext], yogaS[sbyorba], yogaT[sbyorba],
                      byedS[byedpax], byedT[byedpax] );
              }
            else
              {
            sprintf ( outbuf, "%d. %s, %d %s %d. %s/%s; %s/%s, %s/%s.",
                      tt, dayoweek[ (int) dayofweek ], d_d, wmonths[m_m-1],
                       y_y, lunmanS[lunmanx], lunmanT[lunmanx], yogaS[sbyorba], yogaT[sbyorba],
                      byedS[byedpax], byedT[byedpax] );
              }
            fprintf ( fptgt, "%s\n", outbuf );

// LINE 1B: SECOND OF DUPLICATED
            if ( lmchg ) // If lunar mansion changes:
              {
                phrod1 = getphrod ((int) dayofweek, lunmanx, mol1 );
                phrod2 = getphrod ((int) dayofweek, lunmannext, mol1 );
                sprintf ( outbuf, "%s-%s -> %s-%s, %s -> %s.",
                          wdayelem[(int) dayofweek], lmanelem[lunmanx],
                          wdayelem[(int) dayofweek], lmanelem[lunmannext],
                          phrodE[phrod1], phrodE[phrod2] );
              }
            else
              {
                phrod1 = getphrod ((int) dayofweek, lunmanx, mol1 );
                sprintf ( outbuf, "%s-%s, %s.",
                          wdayelem[(int) dayofweek], lmanelem[lunmanx],
                          phrodE[phrod1] );
              }
            fprintf ( fptgt, "\x20\x20%s\n", outbuf );
            
            tenbrelx = get_tenbrel ( newmon[mth_ind].mnthnumK, tt );                   
            get_bishti_str ( tt );             
            
// LINE 2: SECOND OF DUPLICATED
            if ( !bishti_str[0] )
              {
                sprintf ( outbuf, "%s/%s", tenbrelT[tenbrelx], tenbrelE[tenbrelx] );
              }
            else
              {
                sprintf ( outbuf, "%s/%s, %s", tenbrelT[tenbrelx], tenbrelE[tenbrelx], bishti_str );
              }
            fprintf ( fptgt, "\x20\x20%s\n", outbuf );
            
// Extra lines:
            if ( ( ( tt == 14 && tt_chad == 15 ) ||
                ( tt == 29 && tt_chad == 30 ) ) && eclipse_flg )
              {
                fprintf ( fptgt, "\x20\x20\x20\x20%s\n", eclps_str );
              }

            if ( chk_sign ( juldatf, sol ) )  // Check for change of sign
              // Result is put into outbufSC ready for printing.
              SC_flag = 1; // Flag for Sun sign change.

// This only works if 15 or 30 are not omitted days. This should be fixed:
            if ( tt == 15 || tt == 30 )
              {
                NFM_flag = 1; // Flag for new or full Moon days.
                if ( !get_NFMoon ( juldatf, tt ) ) // Error code:
                  {
                    printf ( "Real problem - NO New/Full Moon found\n" );
                    getch ();
                  }
              }
            if ( SC_flag && NFM_flag )
              {
                if ( jul_SC < jul_NFM ) // Sign change goes first:
                  {
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                  }
                else // New/Full Moon goes first.
                  {
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                  }
              }
            else if ( SC_flag || NFM_flag ) // If only one needs to be printed.
              {
                if ( SC_flag )
                  {
                    fprintf ( fptgt, "\x20\x20%s\n", outbufSC );
                  }
                else if ( NFM_flag )
                  {
                    fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbufNFM );
                  }
              }

            if ( NFM_flag && tt_lhag != 1 && eclipse_flg )
              {
                fprintf ( fptgt, "\x20\x20\x20\x20%s\n", eclps_str );
              }
            fprintf ( fptgt, "\x20\x20%s\n", "Duplicated lunar day." );
              } // END - SECOND OF DUPLICATED
            if ( tt_chad && tt_chad != 1 )  // OMITTED TITHI, other than first
              { 
// LINE 1:
                sprintf ( outbuf, "%d. Omitted lunar day.", tt_chad );                      
                fprintf ( fptgt, "%s\n", outbuf );
                if ( d_bug) fprintf ( fptgt_d, "%s\n", outbuf );                                
// LINE 2: (if special)
                if ( !newmon[mth_ind].delaymth &&
                  chk_spec ( newmon[mth_ind].mnthnumK, tt_chad ) )
                  fprintf ( fptgt, "\x20\x20\x20\x20%s\n", outbuf );
                         tt_chad_keep = 0;                
                if ( d_bug )
                  {
                    sprintf ( outbuf, "tt_chad = %d, tt = %d.", tt_chad, tt );
                    fprintf ( fptgt, "%s\n", outbuf );                    
                    fprintf ( fptgt_d, "%s\n", outbuf );
                  } 
                if ( tt_chad == 30 ) 
                  more = 0;                 
              } // END - if ( tt_chad && tt_chad != 1 ) - OMITTED                            
              
// EXTRA LINES - THESE FOLLOW ALL TYPES OF DAY
              
            if ( tt_lhag == 2 )
              tt_lhag = 0;
            chg_tim ( 0, 0, 1, 0, 0 );
              
            if ( tt == 30 && tt_lhag == 0 ) 
              more = 0;

            else if ( tt_lhag == 1 )
              tt_lhag = 2;                                       
            tt_chad = 0;              
            last_tt = tt;
            first_day = 0; // Used as check for omitted first              
          } while ( more ); // END OF DAY LOOP - end of day loop
        last_tt = 0;
      } // END OF MONTH LOOP.
    fclose ( fptgt );
  } // END - prn_cal

// juldat is set to juldatf = time at daybreak at target location.
// Checking for time of new/full Moon. sl = solar, ml = lunar longitudes.
int get_NFMoon ( double juldat, int tt )
  {
    double jd, dif, melong;
    int n, p, b;
    int done = 0;

    sun_mon ( juldat ); // This adds Delta T
// This only works if 15 or 30 are not omitted days. This should be fixed.
    if ( tt == 30 ) // New moon
      {
        melong = mol - sol;
        if ( melong < -330.0 )
          melong = melong + 360.0;
        else if ( melong > 330.0 )
          melong = melong - 360.0;
        jd = juldat;
        do
          {
            jd = jd - ( melong / 12.0 ); // Mean Moon moves 13.2 degrees/day
            if ( jd < juldat || jd > (juldat + 1.0) )
              {
                printf ( "Problem 1 searching for new Moon.\n" );
                getch ();
              }
            sun_mon ( jd );
            melong = mol - sol;

            if ( melong > -0.00001 && melong < 0.00001 )
              done = 1;
            if ( melong < -330.0 )
              melong = melong + 360.0;
            else if ( melong > 330.0 )
              melong = melong - 360.0;
            if ( melong < -15.0 || melong > 15.0 )
              {
                printf ( "ERROR - Sun: %f, Moon: %f\n", sol, mol );
                getch ();
                return (0);
              }
          } while ( !done );

        jul_NFM = jd;
        dif = 60.0 * ( jd - juldat );  // Seems right.
        n = (int) floor ( dif );
        dif = 60.0 * ( dif - (double) n );
        p = (int) floor ( dif );
        dif = 6.0 * ( dif - (double) p );
        b = (int) floor ( dif );
// Convert nadi, pala and breaths from daybreak,
// into hours, minutes and seconds from midnight:        
        conv2hms ( &h, &m, &s, n, p, b );
        sprintf ( outbufNFM, "New Moon at %d:%d:%d (%dh %dm %ds).", n, p, b, h, m, s );
        return (1);
      }
    else if ( tt == 15 ) // Full moon
      {
        melong = mol - sol;
        if ( melong < 0.0 )
          melong = melong + 360.0;
        jd = juldat;
        do
          {
            jd = jd - ( ( melong - 180.0 ) / 12.0 ); // Mean Moon moves 13.2
                                                     // degrees/day
            if ( jd < juldat || jd > (juldat + 1.0) )
              {
                printf ( "Problem 2 searching for full Moon.\n" );
                getch ();
              }
            sun_mon ( jd );
            melong = mol - sol;
            if ( melong < 0.0 )
              melong = melong + 360.0;

            if ( melong > 179.99999 && melong < 180.00001 )
              done = 1;
            if ( melong < 164.0 || melong > 196.0 )
              {
                printf ( "ERROR - Sun: %f, Moon: %f\n", sol, mol );
                getch ();
                return (0);
              }
          } while ( !done );

        jul_NFM = jd;
        dif = 60.0 * ( jd - juldat );  // Seems right.
        n = (int) floor ( dif );
        dif = 60.0 * ( dif - (double) n );
        p = (int) floor ( dif );
        dif = 6.0 * ( dif - (double) p );
        b = (int) floor ( dif );
// Convert nadi, pala and breaths from daybreak,
// into hours, minutes and seconds from midnight:
        conv2hms ( &h, &m, &s, n, p, b );
        sprintf ( outbufNFM, "Full Moon at %d:%d:%d (%dh %dm %ds).", n, p, b, h, m, s );
        return (1);
      }
    return (0); // 0 is error code here, no new or full Moon found.
  } // END - get_NFMoon ()

double getmeanSun ( double juld )
  {
    double T, TM, TM2, TM3, TM4, TM5, X, delt, SunL;

// From sun_mon:
    T = (juld - 2451545.0);
    delt = get_deltaT (juld) / 86400.0; // This converts seconds to fraction of a day
    T = (T + delt) / 36525.0;  // This is now in Julian centuries.

    TM = T / 10.0; // Measured in Julian millenia
    TM2 = TM * TM;
    TM3 = TM * TM2;
    TM4 = TM2 * TM2;
    TM5 = TM * TM4;

// Mean Sun, Astronomical Algorithms, p.183: Should compare with p.212

    X = 280.4664567 + 360007.6982779 * TM;
    X = modfeh ( X, 360.0 );
    SunL = X + 0.03032028 * TM2
             + TM3 / 49931.0
             - TM4 / 15300.0
             - TM5 / 2000000.0;
    SunL = modfeh ( SunL, 360.0 );
    if ( SunL < 0.0 )
      SunL = SunL + 360.0;
    return ( SunL );
  } // END - getmeanSun

double getmeanSunmeannewMoon ( double juld )
  {
    double T, T2, T3, T4, X, delt, SunL;
    double D; // mean elongation of the Moon
    int more = 1;

    do {
         T = (juld - 2451545.0);
         delt = get_deltaT (juld) / 86400.0; // This converts seconds to fraction of a day
         T = (T + delt) / 36525.0;  // This is now in Julian centuries.
         T2 = T * T;
         T3 = T * T2;
         T4 = T2 * T2;

// Mean Moon elongation, Astronomical Algorithms, p.338: 

          X = 297.8501921 + 445267.1114034 * T;
          X = modfeh ( X, 360.0 );
          D = X - 0.0018819 * T2
             + T3 / 545868.0
             - T4 / 113065000.0;
          D = modfeh ( D, 360.0 );
          if ( D > 180.0 )
            D = D - 360.0;
          else if ( D < -180.0 )  
            D = D + 360.0;
          if ( D < 0.00001 && D > -0.00001 ) // order of thousandths of a second!
            more = 0;
          else
            juld = juld - D * ( 29.5 / 360.0 ); 
        } while ( more );
    SunL = getmeanSun ( juld );
    return ( SunL );
  } // END - getmeanSunmeannewMoon

int getphrod ( int dow, int man, double lunlong )
  {
    int x;
// First cater for the difference between 27 & 28 lunar mansion sets
    if ( man > 21 )
      {
        ++man;
      }
    else if ( man == 21 )
      {
        if ( lunlong > 286.666666 )
          man = 22;
      }
    x = man + 32;
    x = x - 4 * dow;
    if ( x < 0 )
      {
        printf ( "ERROR - phrod zero!\n" );
        getch ();
      }
    if ( x > 27 )
      x = x - 28;
    if ( x > 27 )
      x = x - 28;
    return (x);
  } // END - getphrod ()

// This converts Kalacakra time notation in nadi, pala and breaths, from daybreak
// into standard hours, minutes and seconds, from midnight.
void conv2hms ( int *h, int *m, int *s, int n, int p, int b )
  {
    double tim;
    tim = (( (double) n * 60.0 + (double) p ) * 6.0 + (double) b ) / 21600.0;
    tim = tim * 24.0;
    *h = (int) floor (tim) + 5; // Convert to LMST
    tim = tim - floor (tim);
    tim = tim * 60.0;
    *m = (int) floor (tim);
    tim = tim - floor (tim);
    tim = tim * 60.0;
    *s = (int) floor (0.5 + tim);
  } // END - conv2hms ()

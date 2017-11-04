/**
   SolarTracker_Arduino.ino
   Purpose: This program calculate the sun position with Solar Position Algorithm
   by Ibrahim Reda and Afshin Andreas.

   @author   Ya-Liang Chang (Allen)
   @version  0.1 2017-11-2

*/



#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
#include "spa.h" //include the SPA header file
}


//Update period of the solar tracker
#define UPDATE_PEIORD 1800

//2017 constants
#define DELTA_UT1 0.3
#define DELTA_T 69.1

//Pitt Benedum Hall constant
#define TIMEZONE -4 // Pittsburgh
#define LONGITUDE -79.958767 // Pitt Benedum Hall
#define LATITUDE 40.443651 // Pitt Benedum Hall
#define HEIGHT 40 // Pitt Benedum Hall


#ifndef ARDPRINTF
#define ARDPRINTF
#define ARDBUFFER 16 //Buffer for storing intermediate strings. Performance may vary depending on size.
#include <stdarg.h>
#include <Arduino.h> //To allow function to run from any file in a project

int ardprintf(char *str, ...) //Variadic Function
{
  int i, count = 0, j = 0, flag = 0;
  char temp[ARDBUFFER + 1];
  for (i = 0; str[i] != '\0'; i++)  if (str[i] == '%')  count++; //Evaluate number of arguments required to be printed

  va_list argv;
  va_start(argv, count);
  for (i = 0, j = 0; str[i] != '\0'; i++) //Iterate over formatting string
  {
    if (str[i] == '%')
    {
      //Clear buffer
      temp[j] = '\0';
      ardprintf(temp);
      j = 0;
      temp[0] = '\0';

      //Process argument
      switch (str[++i])
      {
        case 'd': Serial.print(va_arg(argv, int));
          break;
        case 'l': Serial.print(va_arg(argv, long));
          break;
        case 'f': Serial.print(va_arg(argv, double));
          break;
        case 'c': Serial.print((char)va_arg(argv, int));
          break;
        case 's': Serial.print(va_arg(argv, char *));
          break;
        default:  ;
      };
    }
    else
    {
      //Add to buffer
      temp[j] = str[i];
      j = (j + 1) % ARDBUFFER;
      if (j == 0) //If buffer is full, empty buffer.
      {
        temp[ARDBUFFER] = '\0';
        ardprintf(temp);
        temp[0] = '\0';
      }
    }
  };

  Serial.println(); //Print trailing newline
  return count + 1; //Return number of arguments detected
}

#undef ARDBUFFER
#endif

void print_help_message() {
  ardprintf("***********************************************\n");
  ardprintf("*  Welcome to SolOasis Solar Tracking System  *\n");
  ardprintf("*                                             *\n");
  ardprintf("*  The SPA library is from 'Solar Position    *\n");
  ardprintf("*  Algorithm for Solar Radiation Applications,*\n");
  ardprintf("*  2008' by Ibrahim Reda and Afshin Andreas   *\n");
  ardprintf("*                                             *\n");
  ardprintf("***********************************************\n\n");
}
void setup()
{
  print_help_message();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  spa_data spa;  //declare the SPA structure
  int result; // SPA result
  float min, sec; // For sunrise/set
  int yy, mm, dd, hr, mmin, ssec; // Desired time to calculate
  float lo = LONGITUDE;
  float la = LATITUDE;
  float hei = HEIGHT;
  char ans[100]; // Answer of yes/no
  char buffer[100]; //for input
  FILE *filePtr; // For recording sun location of the day
  char filename[sizeof "output/20171024_lookup_table.txt"];
  char location_result[100];
  time_t t;
  struct tm * timePtr;


  // Enter required input values into SPA structure
  t = time(0);   // get time now
  timePtr = localtime( & t );
  //Using current time
  yy = timePtr->tm_year + 1900;
  mm = timePtr->tm_mon + 1;
  dd = timePtr->tm_mday;
  hr = timePtr->tm_hour;
  mmin = timePtr->tm_min;
  ssec = timePtr->tm_sec;

  ardprintf("Time: %4d-%02d-%02d %02d:%02d:%02d \n",
            yy, mm, dd, hr, mmin, ssec);
  //f = fopen(("%4d-%02d-%02d %02d:%02d:%02d.txt", yy, mm, dd, hr, mmin, ssec), "ab+");
  // sprintf(filename, "output/%04d%02d%02d_lookup_table.txt", yy, mm, dd);
  // filePtr = fopen(filename, "w");
  ardprintf("Created %s \n\n", filename);

  ardprintf("Using GPS location (%6f, %6f)\n\n", lo, la);


  spa.year          = yy;
  spa.month         = mm;
  spa.day           = dd;
  spa.hour          = hr;
  spa.minute        = mmin;
  spa.second        = ssec;
  spa.timezone      = TIMEZONE;
  spa.delta_ut1     = DELTA_UT1;
  spa.delta_t       = DELTA_T;
  spa.longitude     = lo; //Pittsburgh
  spa.latitude      = la; //Pittsburgh
  spa.elevation     = hei;
  spa.pressure      = 1013.25;
  spa.temperature   = 15;
  spa.slope         = 0;
  spa.azm_rotation  = -10;
  spa.atmos_refract = 0.5667;
  spa.function      = SPA_ALL;

  //call the SPA calculate function and pass the SPA structure

  result = spa_calculate(&spa);

  if (result == 0)  //check for SPA errors
  {
    //display the results inside the SPA structure

    ardprintf("Julian Day:    %.6f\n", spa.jd);
    ardprintf("L:             %.6e degrees\n", spa.l);
    ardprintf("B:             %.6e degrees\n", spa.b);
    ardprintf("R:             %.6f AU\n", spa.r);
    ardprintf("H:             %.6f degrees\n", spa.h);
    ardprintf("Delta Psi:     %.6e degrees\n", spa.del_psi);
    ardprintf("Delta Epsilon: %.6e degrees\n", spa.del_epsilon);
    ardprintf("Epsilon:       %.6f degrees\n", spa.epsilon);
    ardprintf("Zenith:        %.6f degrees\n", spa.zenith);
    ardprintf("Azimuth:       %.6f degrees\n", spa.azimuth);
    ardprintf("Incidence:     %.6f degrees\n", spa.incidence);

    min = 60.0 * (spa.sunrise - (int)(spa.sunrise));
    sec = 60.0 * (min - (int)min);
    ardprintf("Sunrise:       %02d:%02d:%02d Local Time\n", (int)(spa.sunrise), (int)min, (int)sec);

    min = 60.0 * (spa.sunset - (int)(spa.sunset));
    sec = 60.0 * (min - (int)min);
    ardprintf("Sunset:        %02d:%02d:%02d Local Time\n", (int)(spa.sunset), (int)min, (int)sec);

  } else ardprintf("SPA Error Code: %d\n", result);

}

void loop() {
  // put your main code here, to run repeatedly:

}

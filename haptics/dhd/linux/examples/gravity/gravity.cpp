//  (C) 2001-2010 Force Dimension
//  All Rights Reserved.
//
//  Version 3.2.2 ($Rev: 1010 $)



#include <stdio.h>

#include "dhdc.h"

#define REFRESH_INTERVAL  0.1   // sec



int
main (int  argc,
      char **argv)
{
  double t0 = dhdGetTime ();
  double px, py, pz;
  double oa, ob, og;
  double freq = 0.0;
  int done = 0;
  bool rotator = false;

  // message
  int major, minor, release, revision;
  dhdGetAPIVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Gravity Compensation Example %d.%d.%d.%d\n", major, minor, release, revision);
  printf ("(C) 2010 Force Dimension\n");
  printf ("All Rights Reserved.\n\n");

  // open the first available device
  if (dhdOpen () < 0) {
    printf ("error: cannot open device (%s)\n", dhdErrorGetLastStr());
    return -1;
  }

  // identify device
  printf ("%s device detected\n\n", dhdGetSystemName());

  // display instructions
  printf ("press FORCE BUTTON to enable gravity compensation\n");
  printf ("press BUTTON or 'q' to quit\n\n");

  // loop while the button is not pushed
  while (!done) {

    // apply zero force
    if (dhdSetForceAndTorque (0.0, 0.0, 0.0, 0.0, 0.0, 0.0) < DHD_NO_ERROR) {
      printf ("error: cannot set force/torque (%s)\n", dhdErrorGetLastStr());
      done = 1;
    }

    // if necessary, display refresh rate
    if (dhdGetTime () - t0 > REFRESH_INTERVAL) {
      freq = dhdGetComFreq ();
      t0   = dhdGetTime ();

      // write down position
      if (dhdGetPosition (&px, &py, &pz) < 0) {
        printf ("error: cannot read position (%s)\n", dhdErrorGetLastStr());
        done = 1;
      }
      printf ("x:%0.03f y:%0.03f z:%0.03f", px, py, pz);

      // write down orientation (optional)
      if (rotator) {
        if (dhdGetOrientationDeg (&oa, &ob, &og) < 0) {
          printf ("error: cannot read position (%s)\n", dhdErrorGetLastStr());
          done = 1;
        }
        printf ("  a:%0.01f b:%0.01f g:%0.01f", oa, ob, og);
      }

      // clean up
      printf ("   [%0.02f kHz]       \r", freq);

      // test for exit condition
      if (dhdGetButton(0)) done = 1;
      if (dhdKbHit()) {
        if (dhdKbGet() == 'q') done = 1;
      }
    }
  }

  // close the connection
  dhdClose ();

  // happily exit
  printf ("\ndone.\n");
  return 0;
}

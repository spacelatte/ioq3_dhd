//  (C) 2001-2010 Force Dimension
//  All Rights Reserved.
//
//  Version 3.2.2 ($Rev: 1010 $)



#include <stdio.h>

#include "dhdc.h"



int
main (int    argc,
      char **argv)
{
	int i;
  int done = 0;
  int enc[DHD_MAX_ENC];
  int encCount;

  // message
  int major, minor, release, revision;
  dhdGetAPIVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Encoder Reading Example %d.%d.%d.%d\n", major, minor, release, revision);
  printf ("(C) 2010 Force Dimension\n");
  printf ("All Rights Reserved.\n\n");

  // open the first available device
  if (dhdOpen () < 0) {
    printf ("error: cannot open device (%s)\n", dhdErrorGetLastStr());
    return -1;
  }

  // identify device
  printf ("%s device detected\n\n", dhdGetSystemName());

  // identify number of encoders to report based on device type
  switch (dhdGetSystemType ()) {
  case DHD_DEVICE_3DOF:
  case DHD_DEVICE_3DOF_USB:
  case DHD_DEVICE_OMEGA:
  case DHD_DEVICE_OMEGA3:
  default:
    encCount = 3;
    break;
  case DHD_DEVICE_6DOF:
  case DHD_DEVICE_6DOF_500:
  case DHD_DEVICE_6DOF_USB:
  case DHD_DEVICE_OMEGA33:
  case DHD_DEVICE_OMEGA33_LEFT:
    encCount = 6;
    break;
  case DHD_DEVICE_OMEGA331:
  case DHD_DEVICE_OMEGA331_LEFT:
    encCount = 7;
    break;
  case DHD_DEVICE_CONTROLLER:
  case DHD_DEVICE_CONTROLLER_HR:
    encCount = 8;
    break;
  }

  // display instructions
  printf ("press BUTTON or 'q' to quit\n\n");

  // configure device
  dhdEnableExpertMode();

  // loop while the button is not pushed
  while (!done) {

    // read all available encoders
    if (dhdGetEnc (enc) < 0) {
      printf ("error: cannot read encoders (%s)\n", dhdErrorGetLastStr ());
      done = 1;
    }

    // print out encoders according to system type
    for (i=0; i<encCount; i++) {
      printf ("%06d ", enc[i]);
    }
    printf ("          \r");

    // limit to kHz and check for exit condition
    dhdSleep (0.001);
    if (dhdGetButton(0)) done = 1;
    if (dhdKbHit()) {
      if (dhdKbGet() == 'q') done = 1;
    }
  }

  // close the connection
  dhdClose ();

  // happily exit
  printf ("\ndone.\n");
  return 0;
}

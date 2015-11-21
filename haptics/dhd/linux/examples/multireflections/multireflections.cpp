//  (C) 2001-2010 Force Dimension
//  All Rights Reserved.
//
//  Version 3.2.2 ($Rev: 1010 $)



#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
using namespace std;

// identify resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())
string resourceRoot;

#if defined(LINUX) || defined(APPLE)
#include <pthread.h>
#endif

#include "CTexture.h"
#include "CMaths.h"
#include "CMacrosGL.h"

#include "dhdc.h"



const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

// size of torus
const double torusRadius0 = 0.75;
const double torusRadius1 = 0.40;

// size of finger
const double fingerRadius = 0.1;

// status flags
bool simulationOn;
bool simulationFinished;

// texture variables
GLsizei sphereTexW, sphereTexH, padSphereTexW, padSphereTexH;
int sphereTexComp;

// position of finger{s} in global world coordinates
int       fingerCount;
cVector3d fingerPosGlobal[DHD_MAX_DEVICE];

// rotation frame of finger(s) in global world coordinate
cMatrix3d fingerRotGlobal[DHD_MAX_DEVICE];

// position and orientation of torus in global world coordinates
cVector3d torusPosGlobal;
cMatrix3d torusRotGlobal;
cVector3d torusVelGlobal;

// show/hide orientation frame for devices which handle orientation capabilities
bool showFrame = false;

// workspace scale
double scale = 25;

// object stiffness
double stiffness = 1000;



int
init (string sphereFile)
{
  int i;

  glEnable (GL_DEPTH_TEST);

  if (create_texture(sphereFile.c_str(), &sphereTexW, &sphereTexH, &padSphereTexW, &padSphereTexH, &sphereTexComp) < 0) return -1;

  glClearColor (0.0, 0.0, 0.0, 1.0);

  for (i=0; i<fingerCount; i++) fingerPosGlobal[i].zero();
  torusPosGlobal.zero();
  torusRotGlobal.identity();
  torusRotGlobal.rotate(cVector3d(0,1,-1), cDegToRad(45));
  torusVelGlobal.zero();

  return 0;
}



void
rezizeWindow (int w,
              int h)
{
  double glAspect = ((double)w / (double)h);

  glViewport      (0, 0, w, h);
  glMatrixMode    (GL_PROJECTION);
  glLoadIdentity  ();
  gluPerspective  (60, glAspect, 0.01, 10);

  gluLookAt       (3, 0, 0,
                   0, 0, 0,
                   0, 0, 1);

  glMatrixMode    (GL_MODELVIEW);
  glLoadIdentity  ();
}



void
drawScene ()
{
  static const double size = 0.3;

  int            i;
  cMatrixGL      mat;
  GLUquadricObj *sphere;

  // set texture
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexGeni       (GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni       (GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glEnable        (GL_TEXTURE_GEN_S);
  glEnable        (GL_TEXTURE_GEN_T);
  glEnable        (GL_TEXTURE_2D);
  glEnable        (GL_CULL_FACE);
  glTexEnvi       (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // render torus
  mat.set (torusPosGlobal, torusRotGlobal);
  mat.glMatrixPushMultiply ();
  glutSolidTorus (torusRadius1, torusRadius0, 32, 32);
  mat.glMatrixPop ();

  // render fingers
  for (i=0; i<fingerCount; i++) {
    mat.set (fingerPosGlobal[i], fingerRotGlobal[i]);
    mat.glMatrixPushMultiply ();
    sphere = gluNewQuadric ();
    glEnable  (GL_TEXTURE_2D);
    gluSphere (sphere, fingerRadius, 64, 64);

    // render a small frame
    if (showFrame) {
      glDisable   (GL_TEXTURE_2D);
      glDisable   (GL_LIGHTING);
      glColor3f   (1.0, 1.0, 1.0);
      glBegin     (GL_LINES);
      glVertex3d  (0.0, 0.0, 0.0);
      glVertex3d  (size, 0.0, 0.0);
      glVertex3d  (0.0, 0.0, 0.0);
      glVertex3d  (0.0, size, 0.0);
      glVertex3d  (0.0, 0.0, 0.0);
      glVertex3d  (0.0, 0.0, size);
      glEnd ();
    }

    mat.glMatrixPop ();
  }

  // finalize
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_TEXTURE_GEN_S);
  glDisable (GL_TEXTURE_GEN_T);
  glEnable  (GL_LIGHTING);
  glDisable (GL_CULL_FACE);
}



void
draw (void)
{
  GLenum err;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawScene ();

  err = glGetError();
  if (err != GL_NO_ERROR) printf ("error:  %s\n", gluErrorString(err));

  glutSwapBuffers ();
}



void
updateDisplay (int val)
{
  draw ();
  glutTimerFunc (30, updateDisplay, 0);
}



void
key (unsigned char key,
     int           x,
     int           y)
{
  if (key == 27) {
    simulationOn = false;
    while (!simulationFinished) dhdSleep (1);
    exit(0);
  }
}



void
setOther (int value)
{
  switch (value) {
  case OPTION_FULLSCREEN:
    glutFullScreen ();
    break;
  case OPTION_WINDOWDISPLAY:
    glutReshapeWindow      (512, 512);
    glutInitWindowPosition (0, 0);
    break;
  }

  glutPostRedisplay ();
}



void*
hapticsLoop (void* pUserData)
{
  const double increment = 1/3000.0;

  // variables
  int       i;
  int       counter = 0;
  long      t0 = dhdGetSystemCounter();
  double    tdiff;
  double    posX, posY, posZ;
  cVector3d newFingerPosGlobal;
  cVector3d newFingerPosLocal;
  cVector3d forceLocal;
  cVector3d forceGlobal;
  cMatrix3d mat;

  // start haptic simulation
  simulationOn       = true;
  simulationFinished = false;

  // adapt TimeGuard to multidevice: limit refresh rates to 1kHz
  dhdEnableExpertMode ();
  dhdSetTimeGuard (1000,0);
  dhdSetTimeGuard (1000,1);

  // main haptic simulation loop
  while (simulationOn) {

    // compute interaction between torque and each finger
    for (i=0; i<fingerCount; i++) {

      // init variables
      forceGlobal.zero ();

      // read position of haptic device
      dhdGetPosition(&posX, &posY, &posZ, i);

      // read orientation of haptic device
      mat.identity ();
      dhdGetOrientationFrame (mat.m, i);
      fingerRotGlobal[i] = mat;

      // compute position of finger in global world coordinates
      newFingerPosGlobal.set (scale * posX, scale * posY, scale * posZ);

      // compute position of finger in local coordinates of torus
      newFingerPosLocal = cMul (cTrans(torusRotGlobal), cSub(newFingerPosGlobal, torusPosGlobal));

      // project finger on torus plane (z=0)
      cVector3d fingerProjection = newFingerPosLocal;
      fingerProjection.z = 0;

      // search for the nearest point on the torus medial axis
      if (newFingerPosLocal.lengthsq() > CONST_SMALL) {
        cVector3d pointAxisTorus = cMul (torusRadius0, cNormalize(fingerProjection));

        // compute eventual penetration of finger inside the torus
        cVector3d vectTorusFinger = cSub (newFingerPosLocal, pointAxisTorus);

        // finger inside torus, compute forces
        double distance = vectTorusFinger.length();
        if ((distance < (torusRadius1 + fingerRadius)) && (distance > 0.001)) {
          forceLocal = cMul (((torusRadius1 + fingerRadius) - distance) * (stiffness / scale), cNormalize(vectTorusFinger));
          newFingerPosLocal = cAdd (pointAxisTorus, cMul((torusRadius1 + fingerRadius), cNormalize(vectTorusFinger)));
        }

        // finger is outside torus
        else forceLocal.zero();
      }
      else forceLocal.zero();

      // convert reaction force in world coodinates
      forceGlobal = cMul (torusRotGlobal, forceLocal);
      fingerPosGlobal[i] = cMul (torusRotGlobal, newFingerPosLocal);

      // apply forces
      dhdSetForceAndTorque (forceGlobal.x, forceGlobal.y, forceGlobal.z, 0.0, 0.0, 0.0, i);

      // update angular velocity
      torusVelGlobal.add (cMul(-0.01 * increment, cCross(cSub(fingerPosGlobal[i], torusPosGlobal), forceGlobal)));
    }

    // add damping or freeze if any of the devices button is pressed
    for (i=0; i<fingerCount; i++) if (dhdGetButton (0, i)) torusVelGlobal.mul(0.0);
    torusVelGlobal.mul((1 - increment));

    // compute next pose of torus
    if (torusVelGlobal.length() > CONST_SMALL) torusRotGlobal.rotate(cNormalize(torusVelGlobal), (double)(fingerCount) * torusVelGlobal.length());

    // print out refresh rates
    counter++;
    tdiff = (dhdGetSystemCounter()-t0)/1e3;
    if (tdiff > 1000) {
      t0 = dhdGetSystemCounter ();
      printf ("freq: %.2f  ", counter/tdiff);
      for (i=0; i<fingerCount; i++) printf ("[%.2f] ", dhdGetComFreq(i));
      printf (" kHz       \r");
      counter = 0;
    }
  }

  // close connexion with haptic device
  for (i=0; i<fingerCount; i++) dhdClose(i);

  // simulation is now exiting
  simulationFinished = true;

  // return
  return NULL;
}



int
main (int    argc,
      char **argv)
{
  int i;

  // texture file name
  resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);
  string defaultSphereMap = RESOURCE_PATH("spheremap.rgb");   

  // message
  int major, minor, release, revision;
  dhdGetAPIVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Multi-Device Example %d.%d.%d.%d\n", major, minor, release, revision);
  printf ("(C) 2010 Force Dimension\n");
  printf ("All Rights Reserved.\n\n");

  // initialization
  glutInit (&argc, argv);

  // window size and position
  int WINDOW_SIZE_W = 512;
  int WINDOW_SIZE_H = 512;
  int screenW       = glutGet (GLUT_SCREEN_WIDTH);
  int screenH       = glutGet (GLUT_SCREEN_HEIGHT);
  int windowPosX    = (screenW - WINDOW_SIZE_W) / 2;
  int windowPosY    = (screenH - WINDOW_SIZE_H) / 2;

  // create window
  glutInitWindowSize      (WINDOW_SIZE_W, WINDOW_SIZE_H);
  glutInitWindowPosition  (windowPosX, windowPosY);
  glutInitDisplayMode     (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow        (argv[0]);
  glutDisplayFunc         (draw);
  glutKeyboardFunc        (key);
  glutReshapeFunc         (rezizeWindow);
  glutSetWindowTitle      ("Force Dimension - DHD API 3.2");

  // create right-click menu
  glutCreateMenu          (setOther);
  glutAddMenuEntry        ("Full Screen", OPTION_FULLSCREEN);
  glutAddMenuEntry        ("Window Display", OPTION_WINDOWDISPLAY);
  glutAttachMenu          (GLUT_RIGHT_BUTTON);

  // count available haptic devices
  fingerCount = 0;
  if (dhdGetDeviceCount () > 0) {
    for (i=0; i<dhdGetDeviceCount(); i++) {
      if (dhdOpenID (i) == i) {
        fingerCount++;
        printf ("device %d is of type %s\n", i, dhdGetSystemName());
        switch (dhdGetSystemType ()) {
        case DHD_DEVICE_3DOF:
        case DHD_DEVICE_3DOF_USB:
        case DHD_DEVICE_OMEGA:
        case DHD_DEVICE_OMEGA3:
        default:
          showFrame = false;
          break;
        case DHD_DEVICE_6DOF:
        case DHD_DEVICE_6DOF_500:
        case DHD_DEVICE_6DOF_USB:
        case DHD_DEVICE_OMEGA33:
        case DHD_DEVICE_OMEGA33_LEFT:
        case DHD_DEVICE_OMEGA331:
        case DHD_DEVICE_OMEGA331_LEFT:
          showFrame = true;
          break;
        }
      }
    }
  }
  else {
    printf ("failed to open connection to device\n");
    exit (0);
  }

  // generic, multi-device settings
  scale     = 15;
  stiffness = 1500;

  int res = init (defaultSphereMap);

  // tolerate MSVC default runtime directory when running the application from within the IDE
#if defined(WIN32) || defined(WIN64)
  if (res < 0) {
    defaultSphereMap = RESOURCE_PATH("../../../bin/spheremap.rgb");
    res = init (defaultSphereMap);
  }
#endif

  if (res < 0) {
    printf ("cannot find %s\n", defaultSphereMap.c_str());
    return -1;
  }

  // create a high priority haptic thread

#if defined(WIN32) || defined(WIN64)
  DWORD ThreadId;
  CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)(hapticsLoop), NULL, NULL, &ThreadId);
  SetThreadPriority(&ThreadId, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

#if defined(LINUX) || defined(APPLE)
  pthread_t handle;
  pthread_create (&handle, NULL, hapticsLoop, NULL);
  struct sched_param sp;
  memset (&sp, 0, sizeof(struct sched_param));
  sp.sched_priority = 10;
  pthread_setschedparam (handle, SCHED_RR, &sp);
#endif

  // start in window mode
  glutReshapeWindow      (512, 512);
  glutInitWindowPosition (0, 0);

  // update display
  glutTimerFunc (30, updateDisplay, 0);

  // start main graphic rendering loop
  glutMainLoop ();

  return 0;
}

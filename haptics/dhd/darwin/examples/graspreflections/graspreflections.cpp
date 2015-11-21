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

// position of finger in global world coordinates
int       fingerCount = 2;
cVector3d fingerPosGlobal[2];

// position and orientation of torus in global world coordinates
cVector3d torusPosGlobal;
cMatrix3d torusRotGlobal;
cVector3d torusVelGlobal;

// workspace scale
double scale = 1;

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

  // render fingers
  for (i=0; i<fingerCount; i++) {
    mat.set (fingerPosGlobal[i]);
    mat.glMatrixPushMultiply ();
    sphere = gluNewQuadric ();
    gluSphere (sphere, fingerRadius, 64, 64);
    mat.glMatrixPop ();
  }

  // render torus
  mat.set (torusPosGlobal, torusRotGlobal);
  mat.glMatrixPushMultiply ();
  glutSolidTorus (torusRadius1, torusRadius0, 32, 32);
  mat.glMatrixPop ();

  // finalize
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_TEXTURE_GEN_S);
  glDisable (GL_TEXTURE_GEN_T);
  glEnable  (GL_LIGHTING);
  glDisable (GL_CULL_FACE);
}



void
draw(void)
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
  draw();
  glutTimerFunc (30, updateDisplay, 0);
}



void
key (unsigned char key,
     int           x,
     int           y)
{
  if (key == 27) {
    simulationOn = false;
    while (!simulationFinished) dhdSleep(1);
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
  static const double increment = 1/3000.0;

  int       i;
  double    posX, posY, posZ;
  cVector3d newFingerPosGlobal;
  cVector3d newFingerPosLocal;
  cVector3d forceLocal;
  cVector3d forceGlobal[2];

  // start haptic simulation
  simulationOn       = true;
  simulationFinished = false;

  // main haptic simulation loop
  while (simulationOn)
  {
    double    x,y,z;
    cVector3d pos[2];

    dhdGetGripperThumbPos (&x,&y,&z);
    pos[0].set (x,y,z);

    dhdGetGripperFingerPos (&x,&y,&z);
    pos[1].set (x,y,z);

    // compute interaction between torque and each finger
    for (i=0; i<2; i++) {

      // init variables
      forceGlobal[i].zero ();

      // read position of haptic device
      posX = pos[i].x;
      posY = pos[i].y;
      posZ = pos[i].z;

      // compute position of finger in global world coordinates
      newFingerPosGlobal.set (scale * posX, scale * posY, scale * posZ);

      // compute position of finger in local coordinates of torus
      newFingerPosLocal = cMul (cTrans (torusRotGlobal), cSub (newFingerPosGlobal, torusPosGlobal));

      // project finger on torus plane (z=0)
      cVector3d fingerProjection = newFingerPosLocal;
      fingerProjection.z = 0;

      // search for the nearest point on the torus medial axis
      if (newFingerPosLocal.lengthsq () > CONST_SMALL) {
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
      forceGlobal[i]     = cMul (torusRotGlobal, forceLocal);
      fingerPosGlobal[i] = cMul (torusRotGlobal, newFingerPosLocal);

      // update angular velocity
      torusVelGlobal.add (cMul(-0.01 * increment, cCross(cSub(fingerPosGlobal[i], torusPosGlobal), forceGlobal[i])));
    }

    // add damping or freeze if any of the devices button is pressed
    if (dhdGetButton (0)) torusVelGlobal.mul(0.0);
    torusVelGlobal.mul((1.0 - increment));

    // compute next pose of torus
    if (torusVelGlobal.length() > CONST_SMALL) torusRotGlobal.rotate(cNormalize(torusVelGlobal), torusVelGlobal.length());

    // compute projected force
    cVector3d force = cAdd (forceGlobal[0], forceGlobal[1]);
    cVector3d dir   = cSub (pos[1], pos[0]);
    double gripperForce = 0.0;
    if (dir.length() > 0.00001) {
      dir.normalize ();
      cVector3d force = cProject (forceGlobal[1], dir);
      gripperForce = force.length();
      if (force.length() > 0.001) {
        double angle = cAngle(dir, force);
        if ((angle > M_PI/2.0) || (angle < -M_PI/2.0)) gripperForce = -gripperForce;
      }
    }

    // invert if necessary for left-handed devices
    if (dhdIsLeftHanded()) gripperForce = -gripperForce;

    // apply all forces at once
    dhdSetForceAndGripperForce (force.x, force.y, force.z, gripperForce);
  }

  // close connexion with haptic device
  dhdClose ();

  // simulation is now exiting
  simulationFinished = true;

  // return
  return NULL;
}



int
main (int    argc,
      char **argv)
{
  // texture file name
  resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);
  string defaultSphereMap = RESOURCE_PATH("spheremap.rgb");
  
  // message
  int major, minor, release, revision;
  dhdGetAPIVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Haptic Grasping Example %d.%d.%d.%d\n", major, minor, release, revision);
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
  if (dhdOpen () < 0)
  {
    printf ("error: no device found\n");
    exit (-1);
  }

  // identify device
  printf ("%s device detected\n\n", dhdGetSystemName());

  // filter omega.7 only
  switch (dhdGetSystemType()) {
  case DHD_DEVICE_OMEGA331:
  case DHD_DEVICE_OMEGA331_LEFT:
    break;
  default:
    printf ("error: device is not of type omega.7\n");
    exit (-1);
  }

  // omega.7 settings
  scale = 20;
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

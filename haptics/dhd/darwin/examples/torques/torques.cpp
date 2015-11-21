//  (C) 2001-2010 Force Dimension
//  All Rights Reserved.
//
//  Version 3.2.2 ($Rev: 1010 $)



#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "CMaths.h"
#include "CMacrosGL.h"

#if defined(LINUX) || defined(APPLE)
#include <pthread.h>
#endif

#ifdef APPLE
#include "GLUT/glut.h"
#else
#include "GL/glut.h"
#endif

#include "dhdc.h"



// window display options
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

// object stiffness
const double stiffness    = 2000;
const double torque_gain  =    1.0;

// set size of cube (half edge)
const float cube_size = 0.06;

// status flags
bool simulationOn;
bool simulationFinished;
int  status[DHD_MAX_STATUS];
bool hapticsON = false;

// white diffuse light
GLfloat light_ambient[]  = {0.5, 0.5, 0.5, 1.0};
GLfloat light_diffuse[]  = {0.8, 0.8, 0.8, 1.0};
GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};  

// light source position
GLfloat light_position[] = {1.0, 0.5, 0.8, 0.0};  

// normals for the 6 faces of a cube
GLfloat n[6][3]   = { {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
                      {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };

// vertex indices for the 6 faces of a cube
GLint faces[6][4] = { {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
                      {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };

// will be filled in with X,Y,Z vertices
GLfloat v[8][3];  



// graphics initialization
void
initGraphics ()
{
  // compute size of half edge of cube
  static const float size = cube_size / 2.0;

  // setup cube vertex data
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] =  size;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] =  size;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] =  size;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -size;

  // enable a single OpenGL light. 
  glLightfv (GL_LIGHT0, GL_AMBIENT,  light_ambient);
  glLightfv (GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
  glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv (GL_LIGHT0, GL_POSITION, light_position);
  glEnable  (GL_LIGHT0);
  glEnable  (GL_LIGHTING);

  // use depth buffering for hidden surface elimination.
  glEnable (GL_DEPTH_TEST);
}



// window resize callback
void
rezizeWindow (int w,
              int h)
{
  double glAspect = ((double)w / (double)h);

  glViewport     (0, 0, w, h);
  glMatrixMode   (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60, glAspect, 0.01, 10);

  gluLookAt      (0.3, 0.0, 0.0,
                  0.0, 0.0, 0.0,
                  0.0, 0.0, 1.0);

  glMatrixMode   (GL_MODELVIEW);
  glLoadIdentity ();
}



// actual scene rendering function
void
drawScene ()
{
  int            i;
  double         posX, posY, posZ;
  cMatrixGL      mat;
  GLUquadricObj *sphere;
  cVector3d      devicePos;
  cMatrix3d      deviceRot;

  // get info from device
  dhdGetPosition (&posX, &posY, &posZ);
  devicePos.set (posX, posY, posZ);
  dhdGetOrientationFrame (deviceRot.m);

  // render cube
  mat.set (devicePos, deviceRot);
  mat.glMatrixPushMultiply ();

  glEnable  (GL_COLOR_MATERIAL);
  glColor3f (0.1, 0.3, 0.5);

  for (i=0; i<6; i++) {
    glBegin     (GL_QUADS);
    glNormal3fv (&n[i][0]);
    glVertex3fv (&v[faces[i][0]][0]);
    glVertex3fv (&v[faces[i][1]][0]);
    glVertex3fv (&v[faces[i][2]][0]);
    glVertex3fv (&v[faces[i][3]][0]);
    glEnd       ();
  }

  mat.glMatrixPop ();

  // render sphere at center of workspace
  glColor3f (0.8, 0.8, 0.8);
  sphere = gluNewQuadric ();
  gluSphere (sphere, 0.005, 64, 64);
}



// scene rendering callback
void
draw (void)
{
  GLenum err;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawScene ();

  err = glGetError();
  if (err != GL_NO_ERROR) printf("error: %s\n", gluErrorString (err));

  glutSwapBuffers ();
}



// scene rendering timer callback
void
updateDisplay (int val)
{
  draw ();
  glutTimerFunc (30, updateDisplay, 0);
}



// keyboard input callback
void
key (unsigned char key,
     int           x,
     int           y)
{
  if (key == 27) {
    simulationOn = false;
    while (!simulationFinished) dhdSleep (0.01);
    exit(0);
  }
}



// right-click menu callback
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



// collision model
void
computeInteractions (cVector3d& pos,
                     cMatrix3d& rot,
                     cVector3d& force,
                     cVector3d& torque)
{
  static const float size = cube_size / 2.0;

  cMatrix3d localRotTrans;
  cVector3d localPos;
  cVector3d localForce  (0, 0, 0);
  cVector3d localTorque (0, 0, 0);

  // compute position of device in locale coordinates of cube
  rot.transr (localRotTrans);
  localPos = cMul(localRotTrans, cSub (cVector3d (0, 0, 0), pos));

  // compute interaction force and torque
  if ((localPos.x < size) && (localPos.x > -size) &&
      (localPos.y < size) && (localPos.y > -size) &&
      (localPos.z < size) && (localPos.z > -size))
  {
    double    depth = size;
    double    t_depth;
    cVector3d normal (0,0,1);
    cVector3d nForce;

    // check all size walls of cube
    t_depth = fabs( size - localPos.x); if (t_depth < depth) { depth = t_depth; normal.set( 1, 0, 0); }
    t_depth = fabs(-size - localPos.x); if (t_depth < depth) { depth = t_depth; normal.set(-1, 0, 0); }
    t_depth = fabs( size - localPos.y); if (t_depth < depth) { depth = t_depth; normal.set( 0, 1, 0); }
    t_depth = fabs(-size - localPos.y); if (t_depth < depth) { depth = t_depth; normal.set( 0,-1, 0); }
    t_depth = fabs( size - localPos.z); if (t_depth < depth) { depth = t_depth; normal.set( 0, 0, 1); }
    t_depth = fabs(-size - localPos.z); if (t_depth < depth) { depth = t_depth; normal.set( 0, 0,-1); }

    // compute reaction force
    localForce  = cMul(-depth * stiffness, normal);
    nForce      = localForce; nForce.negate();
    localTorque = cMul(-torque_gain, cCross(localPos, nForce));
  }

  // convert results in global coordinates
  force  = cMul(rot, localForce);
  torque = cMul(rot, localTorque);
}



// haptic thread
void*
hapticsLoop (void* pUserData)
{
  double    posX, posY, posZ;
  cVector3d deviceForce;
  cVector3d deviceTorque;
  cVector3d devicePos;
  cMatrix3d deviceRot;

  // start haptic simulation
  simulationOn       = true;
  simulationFinished = false;

  // main haptic simulation loop
  while (simulationOn) {

    // init variables
    posX = posY = posZ = 0.0;

    // read position of haptic device
    dhdGetPosition (&posX, &posY, &posZ);
    devicePos.set  ( posX,  posY,  posZ);

    // read orientation of haptic device
    dhdGetOrientationFrame (deviceRot.m);

    // compute forces and torques
    deviceForce.zero ();
    deviceTorque.zero ();
    computeInteractions (devicePos, deviceRot, deviceForce, deviceTorque);

    // only enable forces once the device if in free space
    if (!hapticsON) {
      if (deviceForce.length() == 0) {
        hapticsON = true;
      }
      else {
        deviceForce.zero();
        deviceTorque.zero();
      }
    }

    // send forces to device
    dhdSetForceAndTorque (deviceForce.x,  deviceForce.y,  deviceForce.z, 
                          deviceTorque.x, deviceTorque.y, deviceTorque.z);
  }

  // close connexion with haptic device
  dhdClose ();

  // simulation is now exiting
  simulationFinished = true;

  // return
  return NULL;
}



// entry point
int
main (int    argc,
      char **argv)
{
  // message
  int major, minor, release, revision;
  dhdGetAPIVersion (&major, &minor, &release, &revision);
  printf ("Force Dimension - Torques Example %d.%d.%d.%d\n", major, minor, release, revision);
  printf ("(C) 2010 Force Dimension\n");
  printf ("All Rights Reserved.\n\n");

  // open the first available device
  if (dhdOpen () < 0) {
    printf ("error: cannot open device (%s)\n", dhdErrorGetLastStr());
    return -1;
  }

  // identify device
  printf ("%s device detected\n\n", dhdGetSystemName());

  // initialization
  glutInit (&argc, argv);
  int WINDOW_SIZE_W = 512;
  int WINDOW_SIZE_H = 512;
  int screenW       = glutGet (GLUT_SCREEN_WIDTH);
  int screenH       = glutGet (GLUT_SCREEN_HEIGHT);
  int windowPosX    = (screenW - WINDOW_SIZE_W) / 2;
  int windowPosY    = (screenH - WINDOW_SIZE_H) / 2;

  // configure window
  glutInitWindowSize     (WINDOW_SIZE_W, WINDOW_SIZE_H);
  glutInitWindowPosition (windowPosX, windowPosY);
  glutInitDisplayMode    (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow       (argv[0]);
  glutDisplayFunc        (draw);
  glutKeyboardFunc       (key);
  glutReshapeFunc        (rezizeWindow);
  glutSetWindowTitle     ("Force Dimension - DHD API 3.2");

  // configure right-click menu
  glutCreateMenu   (setOther);
  glutAddMenuEntry ("Full Screen", OPTION_FULLSCREEN);
  glutAddMenuEntry ("Window Display", OPTION_WINDOWDISPLAY);
  glutAttachMenu   (GLUT_RIGHT_BUTTON);

  // initialize graphics scene
  initGraphics ();

  // create a high priority haptic thread

#if defined(WIN32) || defined(WIN64)
  DWORD ThreadId;
  CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)(hapticsLoop), NULL, NULL, &ThreadId);
  SetThreadPriority(&ThreadId, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

#if defined(LINUX) || defined(APPLE)
  pthread_t          handle;
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


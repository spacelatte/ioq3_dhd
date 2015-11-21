/****************************************************************************
 *
 *  DHD - Haptic API ver 3.2
 *  Copyright (C) 2001-2010
 *  Force Dimension, Switzerland
 *  All Rights Reserved.
 *
 *  contact: support@forcedimension.com
 *
 ****************************************************************************/


/* C header */

#ifndef __DHDC_H__
#define __DHDC_H__


/****************************************************************************
 *  OS DEPENDENCIES
 ****************************************************************************/

#if defined(WIN32) || defined(WIN64)
#ifndef __API
#define __API __stdcall
#endif
#endif

#ifndef __API
#define __API
#endif

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 *  TYPES
 ****************************************************************************/

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

/****************************************************************************
 *  ERROR MANAGEMENT
 ****************************************************************************/

  /* error codes */
  enum dhd_errors {
    DHD_NO_ERROR,
    DHD_ERROR,
    DHD_ERROR_COM,
    DHD_ERROR_DHC_BUSY,
    DHD_ERROR_NO_DRIVER_FOUND,
    DHD_ERROR_NO_DEVICE_FOUND,
    DHD_ERROR_NOT_AVAILABLE,
    DHD_ERROR_TIMEOUT,
    DHD_ERROR_GEOMETRY,
    DHD_ERROR_EXPERT_MODE_DISABLED,
    DHD_ERROR_NOT_IMPLEMENTED,
    DHD_ERROR_OUT_OF_MEMORY,
    DHD_ERROR_DEVICE_NOT_READY,
    DHD_ERROR_FILE_NOT_FOUND,
    DHD_ERROR_CONFIGURATION,
    DHD_ERROR_INVALID_INDEX
  };

  /* types */
  typedef int        *dhdErrorVal;
  typedef const char *dhdErrorStr;

  /* dhd_errno thread-safe global */
  dhdErrorVal __API dhdErrnoLocation ();

#define dhdErrno (*dhdErrnoLocation())

  /* error processing */
  dhdErrorStr __API dhdErrorGetLastStr ();
  dhdErrorStr __API dhdErrorGetStr     (int error);


/****************************************************************************
 *  CONSTANTS
 ****************************************************************************/

/* devices */
#define DHD_DEVICE_NONE            0
#define DHD_DEVICE_3DOF           31
#define DHD_DEVICE_6DOF           61
#define DHD_DEVICE_6DOF_500       62
#define DHD_DEVICE_3DOF_USB       63
#define DHD_DEVICE_6DOF_USB       64
#define DHD_DEVICE_OMEGA          32
#define DHD_DEVICE_OMEGA3         33
#define DHD_DEVICE_OMEGA33        34
#define DHD_DEVICE_OMEGA33_LEFT   36
#define DHD_DEVICE_OMEGA331       35
#define DHD_DEVICE_OMEGA331_LEFT  37
#define DHD_DEVICE_FALCON         60
#define DHD_DEVICE_CONTROLLER     81
#define DHD_DEVICE_CONTROLLER_HR  82
#define DHD_DEVICE_CUSTOM         91

/* status */
#define DHD_ON                     1
#define DHD_OFF                    0

/* device count */
#define DHD_MAX_DEVICE             4

/* encoder count */
#define DHD_MAX_ENC                8

/* motor count */
#define DHD_MAX_MOTOR              6

/* delta motor index */
#define DHD_DELTA_MOTOR_0          0
#define DHD_DELTA_MOTOR_1          1
#define DHD_DELTA_MOTOR_2          2

/* delta encoder index */
#define DHD_DELTA_ENC_0            0
#define DHD_DELTA_ENC_1            1
#define DHD_DELTA_ENC_2            2

/* wrist motor index */
#define DHD_WRIST_MOTOR_0          3
#define DHD_WRIST_MOTOR_1          4
#define DHD_WRIST_MOTOR_2          5

/* wrist encoder index */
#define DHD_WRIST_ENC_0            3
#define DHD_WRIST_ENC_1            4
#define DHD_WRIST_ENC_2            5

/* gripper encoder index */
#define DHD_GRIP_ENC               6
#define DHD_GRIP_MOT               3

/* TimeGuard return value */
#define DHD_TIMEGUARD              1

/* status count */
#define DHD_MAX_STATUS            13

/* status codes */
#define DHD_STATUS_POWER           0
#define DHD_STATUS_CONNECTED       1
#define DHD_STATUS_STARTED         2
#define DHD_STATUS_RESET           3
#define DHD_STATUS_IDLE            4
#define DHD_STATUS_FORCE           5
#define DHD_STATUS_BRAKE           6
#define DHD_STATUS_TORQUE          7
#define DHD_STATUS_WRIST_DETECTED  8
#define DHD_STATUS_ERROR           9
#define DHD_STATUS_GRAVITY        10
#define DHD_STATUS_TIMEGUARD      11
#define DHD_STATUS_ROTATOR_RESET  12

/* buttons count */
#define DHD_MAX_BUTTONS            8

/* velocity estimator computation mode */
#define DHD_VELOCITY_WINDOWING     0
#define DHD_VELOCITY_AVERAGING     1
#define DHD_VELOCITY_WINDOW       20  // [ms]


/****************************************************************************
 *  standard API
 ****************************************************************************/

  int    __API dhdGetDeviceCount                 ();
  int    __API dhdSetDevice                      (char ID);
  int    __API dhdGetDeviceID                    ();
  int    __API dhdGetSerialNumber                (ushort *sn, char ID);
  int    __API dhdOpen                           ();
  int    __API dhdOpenID                         (char ID);
  int    __API dhdClose                          (char ID);
  int    __API dhdStop                           (char ID);
  int    __API dhdGetSystemType                  (char ID);
  int    __API dhdGetVersion                     (double *ver, char ID);
  void   __API dhdGetAPIVersion                  (int *major, int *minor, int *release, int *revision);
  int    __API dhdGetStatus                      (int status[DHD_MAX_STATUS], char ID);
  int    __API dhdGetDeviceAngleRad              (double *angle, char ID);
  int    __API dhdGetDeviceAngleDeg              (double *angle, char ID);
  int    __API dhdGetEffectorMass                (double *mass, char ID);
  ulong  __API dhdGetSystemCounter               ();
  int    __API dhdGetButton                      (int index, char ID);
  bool   __API dhdIsLeftHanded                   (char ID);
  int    __API dhdReset                          (char ID);
  int    __API dhdResetWrist                     (char ID);
  int    __API dhdWaitForReset                   (int timeout, char ID);
  int    __API dhdSetStandardGravity             (double g, char ID);
  int    __API dhdSetGravityCompensation         (int val, char ID);
  int    __API dhdSetBrakes                      (int val, char ID);
  int    __API dhdSetDeviceAngleRad              (double angle, char ID);
  int    __API dhdSetDeviceAngleDeg              (double angle, char ID);
  int    __API dhdSetEffectorMass                (double mass,  char ID);
  int    __API dhdGetPosition                    (double *px, double *py, double *pz, char ID);
  int    __API dhdGetForce                       (double *fx, double *fy, double *fz, char ID);
  int    __API dhdSetForce                       (double  fx, double  fy, double  fz, char ID);
  int    __API dhdGetOrientationRad              (double *oa, double *ob, double *og, char ID);
  int    __API dhdGetOrientationDeg              (double *oa, double *ob, double *og, char ID);
  int    __API dhdGetTorque                      (double *ta, double *tb, double *tg, char ID);
  int    __API dhdSetTorque                      (double  ta, double  tb, double  tg, char ID);
  int    __API dhdGetPositionAndOrientationRad   (double *px, double *py, double *pz, double *oa, double *ob, double *og, char ID);
  int    __API dhdGetPositionAndOrientationDeg   (double *px, double *py, double *pz, double *oa, double *ob, double *og, char ID);
  int    __API dhdGetPositionAndOrientationFrame (double *px, double *py, double *pz, double matrix[3][3], char ID);
  int    __API dhdGetForceAndTorque              (double *fx, double *fy, double *fz, double *ta, double *tb, double *tg, char ID);
  int    __API dhdSetForceAndTorque              (double  fx, double  fy, double  fz, double  ta, double  tb, double  tg, char ID);
  int    __API dhdGetOrientationFrame            (double matrix[3][3], char ID);
  int    __API dhdGetGripperAngleDeg             (double *a, char ID);
  int    __API dhdGetGripperAngleRad             (double *a, char ID);
  int    __API dhdGetGripperThumbPos             (double *px, double *py, double *pz,  char ID);
  int    __API dhdGetGripperFingerPos            (double *px, double *py, double *pz,  char ID);
  int    __API dhdSetGripperTorque               (double t, char ID);
  int    __API dhdSetGripperForce                (double f, char ID);
  double __API dhdGetComFreq                     (char ID);
  int    __API dhdSetForceAndGripperForce        (double fx, double fy, double fz, double f, char ID);
  int    __API dhdSetForceAndGripperTorque       (double fx, double fy, double fz, double t, char ID);
  int    __API dhdConfigLinearVelocity           (int ms, int mode, char ID);
  int    __API dhdGetLinearVelocity              (double *vx, double *vy, double *vz, char ID);
  int    __API dhdEmulateButton                  (uchar val, char ID);


/****************************************************************************
 *  expert API
 ****************************************************************************/

  int    __API dhdEnableExpertMode            ();
  int    __API dhdDisableExpertMode           ();
  int    __API dhdPreset                      (int val[DHD_MAX_ENC], uchar mask, char ID);
  int    __API dhdEnableForce                 (uchar val, char ID);
  int    __API dhdCalibrateWrist              (char ID);
  int    __API dhdSetTimeGuard                (int us,  char ID);
  int    __API dhdSetVelocityThreshold        (uchar val, char ID);
  int    __API dhdGetVelocityThreshold        (uchar *val, char ID);
  int    __API dhdUpdateEncoders              (char ID);
  int    __API dhdGetDeltaEncoders            (int *enc0, int *enc1, int *enc2, char ID);
  int    __API dhdGetWristEncoders            (int *enc0, int *enc1, int *enc2, char ID);
  int    __API dhdGetGripperEncoder           (int *enc, char ID);
  int    __API dhdGetEncoder                  (int index, char ID);
  int    __API dhdSetMotor                    (int index, short val, char ID);
  int    __API dhdSetDeltaMotor               (short mot0, short mot1, short mot2, char ID);
  int    __API dhdSetWristMotor               (short mot0, short mot1, short mot2, char ID);
  int    __API dhdSetGripperMotor             (short mot, char ID);
  int    __API dhdDeltaEncoderToPosition      (int  enc0, int  enc1, int  enc2, double *px, double *py, double *pz, char ID);
  int    __API dhdDeltaPositionToEncoder      (double px, double py, double pz, int  *enc0, int  *enc1, int  *enc2, char ID);
  int    __API dhdDeltaMotorToForce           (short mot0, short mot1, short mot2, int enc0, int enc1, int enc2, double  *fx, double  *fy, double  *fz, char ID);
  int    __API dhdDeltaForceToMotor           (double  fx, double  fy, double  fz, int enc0, int enc1, int enc2, short *mot0, short *mot1, short *mot2, char ID);
  int    __API dhdWristEncoderToOrientation   (int  enc0, int  enc1, int  enc2, double *oa, double *ob, double *og, char ID);
  int    __API dhdWristOrientationToEncoder   (double oa, double ob, double og, int  *enc0, int  *enc1, int  *enc2, char ID);
  int    __API dhdWristMotorToTorque          (short mot0, short mot1, short mot2, int enc0, int enc1, int enc2, double  *ta, double  *tb, double  *tg, char ID);
  int    __API dhdWristTorqueToMotor          (double  ta, double  tb, double  tg, int enc0, int enc1, int enc2, short *mot0, short *mot1, short *mot2, char ID);
  int    __API dhdGripperEncoderToOrientation (int enc, double *a, char ID);
  int    __API dhdGripperEncoderToPosition    (int enc, double *p, char ID);
  int    __API dhdGripperOrientationToEncoder (double a, int *enc, char ID);
  int    __API dhdGripperPositionToEncoder    (double p, int *enc, char ID);
  int    __API dhdGripperMotorToTorque        (short mot, double *t, char ID);
  int    __API dhdGripperMotorToForce         (short mot, double *f, char ID);
  int    __API dhdGripperTorqueToMotor        (double t, short *mot, char ID);
  int    __API dhdGripperForceToMotor         (double f, short *mot, char ID);
  int    __API dhdSetOperatingMode            (unsigned char  mode, char ID);
  int    __API dhdGetOperatingMode            (unsigned char *mode, char ID);


/****************************************************************************
 *  controller API
 ****************************************************************************/

  int    __API dhdControllerSetDevice   (int device, char ID);
  int    __API dhdReadConfigFromFile    (char *filename, char ID);
  int    __API dhdSetMot                (short mot[DHD_MAX_ENC], uchar mask, char ID);
  int    __API dhdGetEnc                (int   enc[DHD_MAX_ENC], uchar mask, char ID);
  int    __API dhdSetBrk                (uchar mask, char ID);


/****************************************************************************
 *  OS independent utilities
 ****************************************************************************/

  bool   __API dhdKbHit   ();
  char   __API dhdKbGet   ();
  double __API dhdGetTime ();
  void   __API dhdSleep   (double sec);


/****************************************************************************
 *  added in 3.2.x SDK
 ****************************************************************************/

  const char* __API dhdGetSystemName (char ID);


#ifdef __cplusplus
}
#endif


#endif

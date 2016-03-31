// added by mert akengin
// haptic interface for q3

//#define _HAPTICS_C_

#include <dhd.h>
#include <haptics.h>
#include <math.h>

#ifndef __cplusplus
	#define bool char
	#define false 0
	#define true 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

//static cvar_t *in_haptic = NULL;
haptic_t *buf = NULL;
unsigned cnt = 0;
static double axes[AXES];
static bool haptic_ok = false;
const double _fixes[AXES] = {2,1,2};
double haptic_offsets[AXES] = {0,0,0};
static double lt = 0;

static const int _vf[AXES] = {2,1,0};
static const float _movfix[AXES] = {-0.4,-0.1,0};
const double _dz[AXES][2] = {
	{0,0},
	{0.003,-0.003},
	{0.005,-0.001}
};

#include <qcommon.h>
void haptic_init()
{
	int i,major, minor, release, revision;
	dhdGetAPIVersion (&major, &minor, &release, &revision);
	Com_Printf ("Force Dimension - %d.%d.%d.%d\n", major, minor, release, revision);
	Com_Printf ("(C) 2010 Force Dimension\n");
	Com_Printf ("All Rights Reserved.\n\n");
	//Com_Printf ("%s device detected\n\n", dhdGetSystemName());
	if(haptic_ok)
		return;
	//axes = (double*)malloc(AXES*sizeof(double));
	buf = (haptic_t*)malloc(sizeof(haptic_t));
	{
		buf->fmove = 0;
		for(i=0;i<AXES;i++)
			buf->views[i] = 0;
		for(i=0;i<BTN_CNT;i++)
			buf->buttons[i] = 0;
	}
	for(i=0;i<AXES;i++)
		axes[i] = 0;
	//haptic_ok = !(dhdOpen() < 0);
	i = dhdOpen();
	if(i >= 0)
		haptic_ok = true;
	Com_Printf("%s\r\n",dhdErrorGetLastStr());
	if(!haptic_ok)
		return;
	//dhdReset(DHD);
//	dhdWaitForReset(0,DHD);
	dhdSetGravityCompensation(DHD_ON,DHD);
	dhdSetForce(0,0,0,DHD);
	return;
}

void haptic_close()
{
	if(!haptic_ok)
		return;
	dhdClose(DHD);
	haptic_ok = false;
	//free(buf);
	buf = NULL;
	return;
}

#define RI 0.01
void haptic_getpos(double *arr)
{
	int i;
	if(!haptic_ok)
		return;
	if(arr == NULL)
		return;
	if(dhdGetTime() - lt < RI)
	{
		for(i=0;i<AXES;i++)
			arr[i] = 0;
		return;
	}else
		lt = dhdGetTime();
	dhdGetPosition(&arr[0],&arr[1],&arr[2],DHD);
	return;
}

void haptic_force(double forces[AXES])
{
	int i;
	if(!haptic_ok)
		return;
	if(forces == NULL)
		return;
	if(abs(forces[i]) > 222)
	{
		dhdSetForce(0,0,0,DHD);
		return;
	}
	dhdSetForce(forces[0],forces[1],forces[2],DHD);
	return;
}

char haptic_btn(void)
{
	char res = 0;
	int i;
	if(!haptic_ok)
		return -1;
	for(i=0;i<BTN_CNT;i++)
		res |= (dhdGetButton(i,DHD) != 0) << i;
	return res;
}

void haptic_fire(unsigned f)
{
	if(!haptic_ok)
		return;
	dhdSetForce(f*10,f*10,f*10,DHD);
	dhdSleep(0.05);
	dhdSetForce(0,0,0,DHD);
	return;
}

void haptic_move(float *angles, double m)
{
	int i;
	if(!haptic_ok)
		return;
	if(angles == NULL)
		return;
	for(i=0;i<AXES-1;i++)
		//if(axes[_vf[i]] > _dz[_vf[i]][0] || axes[_vf[i]] < _dz[_vf[i]][1])
			angles[i] -= (float)(m*angles[_vf[i]]);
	// + _movfix[i];
	return;
}

char haptic_press(char btn)
{
	if(!haptic_ok)
		return -1;
	return (dhdGetButton(btn,DHD));
}

void haptic_wait(float f)
{
	if(!haptic_ok)
		return;
	if(f < 0)
		dhdWaitForReset(0,DHD);
	else
		dhdSleep(f);
	return;
}

void haptic_print(double *arr, const char *tag)
{
	int i;
	if(!haptic_ok)
		return;
	if(arr == NULL)
		return;
	if(tag != NULL)
		Com_Printf("%s: ",tag);
	for(i=0;i<AXES;i++)
		Com_Printf("% 12.9lf ",arr[i]);
	Com_Printf("\r\n");
	return;
}

float haptic_axis(char axis, float m)
{
	double xyz[AXES];
	if(!haptic_ok)
		return 0;
	dhdGetPosition(&xyz[0],&xyz[1],&xyz[2],DHD);
	return (float)(xyz[axis%AXES] * m);
}

void haptic_joystick(double *arr, unsigned p)
{
	int i;
	if(!haptic_ok)
		return;
	if(arr == NULL)
		return;
	for(i=0;i<AXES;i++)
		arr[i] = -pow(arr[i]*(250.0),1);
	//+ _fixes[i];
	arr[0] += (double)(p*2.0);
	return;
}

void haptic_getf(double *arr)
{
	int i;
	if(!haptic_ok)
		return;
	if(arr == NULL)
		return;
	dhdGetForce(&arr[0],&arr[1],&arr[2],DHD);
	return;
}

#include <client.h>
void haptic_dealwith(cvar_t **arr, usercmd_t *cmd, float *va, void *btns)
{
	int i;
	if(!arr || !arr[0] || !(arr[0]->value) || !cmd || !va || !btns || va == NULL)
		return;
	if(!haptic_ok)
		return;
	haptic_getpos(axes);
	if(axes[0] == 0 && axes[1] == 0 && axes[2] == 0)
		return;
	{
		//char fire = haptic_press(2);
		//char jump = haptic_press(0);
		for(i=0;i<BTN_CNT;i++)
			((kbutton_t*)btns)[i].active = (qboolean)(haptic_btn() == 1<<i);
		if(cnt > 0)
			cnt -= 1;
		if(((kbutton_t*)btns)[0].active && cnt < 5)
			cnt += (rand()%5)+3;
		if(!arr[1] || !arr[2])
			return;
		haptic_move(va,(arr[2]->value)*15.0);
		//in_speed.active = qtrue;
		if(arr[1]->value)
		{
			double Y = axes[0]*100;
			if(Y < 1.0 && Y > -1.0)
				Y = 0;
			else
				Y *= -32.0;
			cmd->forwardmove = ClampChar(Y);
		}
		//if(!(axes[1] < -0.045 && axes[1] > 0.046 && axes[2] < -0.045 && axes[2] > 0.054))
		{
			haptic_joystick(axes,cnt);
			haptic_force(axes);
		}
	}
	return;
}

void haptic_stuff(cvar_t sens)
{
	int i;
	if(!haptic_ok)
		return;
	haptic_getpos(axes);
	if(axes[0] == 0 && axes[1] == 0 && axes[2] == 0)
		return;
	for(i=0;i<BTN_CNT;i++)
		buf->buttons[i] = (char)(haptic_btn() == 1<<i);
	if(cnt > 0)
		cnt -= 1;
	if(buf->buttons[0] && cnt < 5)
		cnt += (rand()%5)+3;
	for(i=0;i<AXES-1;i++)
		buf->views[i] = (float)axes[i];
	haptic_move(buf->views,(sens.value)*15.0);
	double Y = axes[0]*100;
	if(Y < 1.0 && Y > -1.0)
		Y = 0;
	else
		Y *= -32.0;
	buf->fmove = ClampChar(Y);
	haptic_joystick(axes,cnt);
	//haptic_force(axes);
	dhdSetForce(axes[0],axes[1],axes[2],DHD);
	return;
}

#ifdef __cplusplus
}
#endif

// mert akengin

#ifndef _HAPTICS_H_
#define _HAPTICS_H_

#define AXES 3
#define BTN_CNT 4
#define DHD -1

#ifdef __cplusplus
extern "C" {
#endif

	#include <q_shared.h>

	typedef struct {
		float views[AXES];
		char buttons[BTN_CNT];
		char fmove;
	} haptic_t;

	extern haptic_t *buf;

	extern unsigned cnt;
	extern double axes[AXES];
	extern const double _fixes[AXES];
	extern double haptic_offsets[AXES];

	void haptic_init(void);
	void haptic_close(void);
	void haptic_getpos(double*);
	void haptic_force(double*);
	char haptic_btn(void);
	void haptic_fire(unsigned);
	void haptic_move(float*,double);
	char haptic_press(char);
	void haptic_wait(float);
	void haptic_print(double*,const char*);
	float haptic_axis(char,float);
	void haptic_joystick(double*,unsigned);
	void haptic_getf(double*);
	void haptic_dealwith(cvar_t**,usercmd_t*,float*,void*);
	void haptic_stuff(cvar_t);

#ifdef __cplusplus
}
#endif
#endif

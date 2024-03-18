#include "../qcommon/qcommon.h"

extern dvar_t* cg_xui_fps;
extern dvar_t* cg_xui_fps_x;
extern dvar_t* cg_xui_fps_y;

#define	FPS_FRAMES 4
void CG_DrawFPS(float y) {
	if (cg_xui_fps->current.integer) {

		static int previousTimes[FPS_FRAMES];
		static int index;
		int	i, total;
		int	fps;
		static int previous;
		int	t, frameTime;

		t = timeGetTime();
		frameTime = t - previous;
		previous = t;
		previousTimes[index % FPS_FRAMES] = frameTime;
		index++;

		if (index > FPS_FRAMES) {
			total = 0;
			for (i = 0; i < FPS_FRAMES; i++) {
				total += previousTimes[i];
			}
			if (!total) {
				total = 1;
			}
			fps = 1000 * FPS_FRAMES / total;

			SCR_DrawSmallStringExt(cg_xui_fps_x->current.value, cg_xui_fps_y->current.value,va("FPS: %d", fps), vColorWhite);
		}
	}
	else {
		void(*call)(float);
		*(int*)&call = 0x4C7020;
		call(y);
	}
}
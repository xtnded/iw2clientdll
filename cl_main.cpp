#include "stdheader.h"
#include "gl33.h"
//#pragma comment(lib, "opengl32.lib")
#include "imgui.h"
#include "cg_public.h"
#include "chakracore.h"

dvar_t *con_restricted = (dvar_t*)0x5E132C;
int *cls_keyCatchers = (int*)0x96B654;
#define KEYCATCH_CONSOLE (1)

void Cmd_Moto_f()
{
	Com_Printf("Running CoD 2 1.4");
}

void Cbuf_AddText_(const char *s)
{
	__asm
	{
		mov eax, s
		lea edx, Cbuf_AddText
		call edx
	}
}

void Cmd_Test_f()
{
	Com_Printf("var = %s", Dvar_GetVariantString("shortversion")->current.string);
	Cbuf_AddText_("say \"hello world\"");
}
#if 1
// Parameters: 

// *filename = file path, 

// width, height = screenshot dimensions (resolution)

// *BMP_Data = BMP data :) 


int SaveBMP(char *filename, int width, int height, unsigned char *BMP_Data)
{
	FILE *file_ptr;
	BITMAPINFOHEADER bmp_infoh;
	BITMAPFILEHEADER bmp_fileh;
	unsigned int counter;
	unsigned char tempRGB;

	file_ptr = fopen(filename, "wb");
	if (!file_ptr)
	{
		return 0;
	}

	bmp_fileh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmp_fileh.bfReserved1 = 0;
	bmp_fileh.bfReserved2 = 0;
	bmp_fileh.bfSize = sizeof(BITMAPFILEHEADER);
	bmp_fileh.bfType = 0x4D42;

	bmp_infoh.biBitCount = 24;
	bmp_infoh.biClrImportant = 0;
	bmp_infoh.biClrUsed = 0;
	bmp_infoh.biCompression = BI_RGB;
	bmp_infoh.biHeight = height;
	bmp_infoh.biPlanes = 1;
	bmp_infoh.biSize = sizeof(BITMAPINFOHEADER);
	bmp_infoh.biSizeImage = width * abs(height) * 3;
	bmp_infoh.biWidth = width;
	bmp_infoh.biXPelsPerMeter = 0;
	bmp_infoh.biYPelsPerMeter = 0;

	for (counter = 0; counter < bmp_infoh.biSizeImage; counter += 3)
	{
		tempRGB = BMP_Data[counter];
		BMP_Data[counter] = BMP_Data[counter + 2];
		BMP_Data[counter + 2] = tempRGB;
	}

	fwrite(&bmp_fileh, 1, sizeof(BITMAPFILEHEADER), file_ptr);
	fwrite(&bmp_infoh, 1, sizeof(BITMAPINFOHEADER), file_ptr);
	fwrite(BMP_Data, 1, bmp_infoh.biSizeImage, file_ptr);

	fclose(file_ptr);
	return 1;
}

// Shoots the screen :)

// Parameters:

// - width, height = screenshot dimensions (resolution)

// Screenshot is named "Screenshot.bmp"


void *BMPw_Data; // BMP Data

void SaveScreenshot(int width, int height)
{
	BMPw_Data = malloc(width * height * 3);
	memset(BMPw_Data, 0, width * height * 3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, BMPw_Data);
	SaveBMP((char*)"screenshot_cod2.bmp", width, height, (unsigned char*)BMPw_Data);

	free(BMPw_Data);
}
#endif
void Cmd_Screenshot_f()
{
	SaveScreenshot(1024, 768);
}

void Cmd_Hook_f()
{

	void patch_opcode_glbindtexture(void);
	patch_opcode_glbindtexture();
}

void handleImGuiWindows()
{
	static char serverCommand[4096] = { 0 };

	if (ImGui::Begin("haha"))
	{

		ImGui::InputText("server cmd", serverCommand, sizeof(serverCommand));
		if (ImGui::Button("send"))
		{
			Cbuf_AddText(serverCommand);
		}
		ImGui::End();
	}
}

void SetWndCapture(bool);

//our own kind of cl_frame to handle things each frame / tick
void CL_Frame()
{
	void(*o)() = (void(*)())0x040F850;
	o();

	extern bool preventMouseGrab;
	bool prev = preventMouseGrab;
	preventMouseGrab = (*cls_keyCatchers & KEYCATCH_CONSOLE) == KEYCATCH_CONSOLE;
	if (prev)
	{
		if (!preventMouseGrab)
		{
			//closing console
			SetWndCapture(false);
		}
	}
	else
	{
		if(preventMouseGrab)
			SetWndCapture(true);
	}
	//Com_Printf("frame!\n");
}

//bool cl_inited = false;

int Com_PrintString(const char *str) {
	Com_Printf("%s", str);
	return 0;
}

void CL_Init( void )
{
	void(*o)(void) = (void(*)(void))0x411650;

	o();

	Cmd_AddCommand("hook", Cmd_Hook_f);

	Dvar_SetFromStringByName("moto", "yes");
	Cmd_AddCommand("moto", Cmd_Moto_f);
	Cmd_AddCommand("test", Cmd_Test_f);
	Cmd_AddCommand("ss", Cmd_Screenshot_f);
	Com_Printf(MOD_NAME " loaded!\n");

	Com_Printf("^6bla: %d\n", GetCurrentThreadId());
	//cl_inited = true;

	CG_InitConsoleCommands();

	chakracore_prepare(Com_PrintString);
	chakracore_eval("console.log(\"^3Loaded ChakraCore\");");
}
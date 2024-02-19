#include "../qcommon/qcommon.h"
#include "../gl33/gl33.h"
#include "../detours/detours.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl2.h"
#include "../imgui/imgui_impl_win32.h"

#if 1
void drawCube(float *org, float size) {

	glPushMatrix();
	glTranslatef(org[0], org[1], org[2]);

	glBegin(GL_QUADS);
	// front face
	glNormal3f(0, 0, 1);
	glColor3f(1.0, 0.0, 0.0);
	glTexCoord2d(0, 0);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);
	// left face
	glNormal3f(-1, 0, 0);
	glColor3f(0.0, 1.0, 0.0);
	glTexCoord2d(0, 1);
	glVertex3f(-size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);
	// back face
	glNormal3f(0, 0, -1);
	glColor3f(0.0, 0.0, 1.0);
	glTexCoord2d(1, 0);
	glVertex3f(size / 2, size / 2, -size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);
	// right face
	glNormal3f(1, 0, 0);
	glColor3f(1.0, 1.0, 0.0);
	glTexCoord2d(1, 1);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	// top face
	glNormal3f(0, 1, 0);
	glColor3f(1.0, 0.0, 1.0);
	glTexCoord2d(0, 0);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	// bottom face
	glNormal3f(0, -1, 0);
	glColor3f(0.0, 1.0, 1.0);
	glTexCoord2d(1, 0);
	glVertex3f(size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glEnd();

	glPopMatrix();
}
#endif

void(WINAPI *orig_glBindTexture)(GLenum target, GLuint texture);
void WINAPI hglBindTexture(GLenum target, GLuint texture) {
	//orig_glBindTexture(target,texture);
	//float org[] = { 1,2,3 };
	//drawCube(org, 10);
	//Com_Printf("glBindTexture!\n");
	orig_glBindTexture(target, 0);
}

void load_imgui()
{
	static bool loaded = false;
	if (loaded)return;
	loaded = true;
	HWND hWnd = FindWindowA(NULL, "Call of Duty 2 Multiplayer");
	if (!hWnd) {
		MessageBoxA(0, "failed to find wnd!", "", 0);
		exit(0);
	}
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplOpenGL2_Init();

	ImGui::StyleColorsDark();
}

#if 0
#define glCheck(x) \
Com_Printf("%s\n", #x); \
x; \
Com_Printf("Error: %02X\n", glGetError());
#else
#define glCheck(x) x
#endif

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";
int shaderProgram;
void load_shaders()
{
	static bool load = false;
	if (load)return;
	load = true;
	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glCheck(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
	glCheck(glCompileShader(vertexShader));
	// check for shader compile errors
	int success;
	char infoLog[512];
	glCheck(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		glCheck(glGetShaderInfoLog(vertexShader, 512, NULL, infoLog));
		Com_Printf("vertex shader compilation failed %s\n", infoLog);
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glCheck(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
	glCheck(glCompileShader(fragmentShader));
	// check for shader compile errors
	glCheck(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		glCheck(glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog));
		Com_Printf("fragment shader compilation failed %s\n", infoLog);
	}
	// link shaders
	shaderProgram = (glCreateProgram());
	glCheck(glAttachShader(shaderProgram, vertexShader));
	glCheck(glAttachShader(shaderProgram, fragmentShader));
	glCheck(glLinkProgram(shaderProgram));
	// check for linking errors
	glCheck(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
	if (!success) {
		glCheck(glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog));
		Com_Printf("shader program linking failed %s\n", infoLog);
	}
	glCheck(glDeleteShader(vertexShader));
	glCheck(glDeleteShader(fragmentShader));
}

unsigned int VBO, VAO;
void setup_verts()
{
	static bool load = false;
	if (load)return;
	load = true;
	float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
	};

	for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]); i++)
	{
		vertices[i] *= 1000.f;
	}

	glCheck(glGenVertexArrays(1, &VAO));
	glCheck(glGenBuffers(1, &VBO));
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glCheck(glBindVertexArray(VAO));

	glCheck(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	glCheck(glEnableVertexAttribArray(0));

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glCheck(glBindVertexArray(0));
}

BOOL(WINAPI *oSwapBuffers)(HDC);
BOOL __stdcall hSwapBuffers(HDC hdc) {
		//Com_Printf("^6swapping buffers!\n");
#if 1
	//glClearColor(1.f,0,0, 1);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
#if 0
	GLint id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);
	GLint current_vao;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
#endif
#if 0
#define H 100
#define W 100
	unsigned int data[H][W][3];
	for (size_t y = 0; y < H; ++y)
	{
		for (size_t x = 0; x < W; ++x)
		{
			data[y][x][0] = (rand() % 256) * 256 * 256 * 256;
			data[y][x][1] = (rand() % 256) * 256 * 256 * 256;
			data[y][x][2] = (rand() % 256) * 256 * 256 * 256;
		}
	}

	glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_INT, data);
#endif
	//load_shaders();
	//setup_verts();
#if 0
	glCheck(glUseProgram(shaderProgram));
	glCheck(glBindVertexArray(VAO)); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 3));
	glBindVertexArray(0); // no need to unbind it every time 
	glUseProgram(0);
#endif
	//glBindVertexArray(current_vao);
	//glUseProgram(id);
#if 1

#if 0
		load_imgui();

			ImGui_ImplWin32_NewFrame();
			ImGui_ImplOpenGL2_NewFrame();

			ImGui::NewFrame();
			bool bShow = true;
			ImGui::ShowDemoWindow(&bShow);
			ImGui::EndFrame();

			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#endif
#endif

			//Com_Printf("gl error = %d\n", glGetError());
			static bool do_once = false;
			if (!do_once)
			{
				Com_Printf("^6bla: %d\n", GetCurrentThreadId());
				//const char *ver = (const char*)glGetString(GL_VERSION);
				//MessageBoxA(NULL, ver, "", 0);
				do_once = true;
			}
	return oSwapBuffers(hdc);
}

FARPROC get_gl_func_ptr(const char *name)
{

	HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
	if (!hOpenGL)
	{
		MessageBoxA(NULL, "no opengl!", 0, 0);
		return 0;
	}
	FARPROC bindtex = GetProcAddress(hOpenGL, name);
	if (!bindtex)
	{
		MessageBoxA(NULL, va("no !",name), 0, 0);
		return 0;
	}
	return bindtex;
}

void patch_opcode_glbindtexture(void)
{
	/*
	int from = 0x4634AC;
	DWORD tmp;
	VirtualProtect((void*)from, 6, PAGE_EXECUTE_READWRITE, &tmp);
	*(BYTE*)(from) = 0xbf;
	*(int*)(from + 1) = (int)hLoadLibraryA;
	*(BYTE*)(from + 5) = 0x90;
	VirtualProtect((void*)from, 6, tmp, &tmp);
	*/
	//load_imgui();

	//wglSwapBuffers
#if 0
	qglReadPixels = (void(__stdcall*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*))get_gl_func_ptr("glReadPixels");
	qglBegin = (void(__stdcall*)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,GLvoid*))get_gl_func_ptr("glBegin");
#endif

	//MessageBoxA(NULL, va("hooking %02 to custom", bindtex), 0, 0);
#if 0
	orig_glBindTexture = (void(__stdcall*)(GLenum target, GLuint texture)) \
		DetourFunction((LPBYTE)get_gl_func_ptr("glBindTexture"), (LPBYTE)hglBindTexture);
#endif
	oSwapBuffers = (BOOL(__stdcall*)(HDC)) \
		DetourFunction((LPBYTE)get_gl_func_ptr("wglSwapBuffers"), (LPBYTE)hSwapBuffers);
}
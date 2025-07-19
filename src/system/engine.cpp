
#if defined(_WINDOWS)

#include "system/extern_inc.h"
#include "system/engine.h"
#include "system/engine_messages.h"
#include "core/input.h"
#include "system/debug.h"
#include "render/context.h"
#include "render/material_manager.h"
#include "render/gui/gui_manager.h"
#include "render/gui/gui_draw.h"
#include "util/stringn.h"
#include "render/gui/gui.h"
#include "render/gui/gui_manager.h"
#include "core/object.h"
#include "core/system.h"

#define GAFFE_SUSPEND_RESUME 0

#define USE_PERFORMANCE_COUNT 1
#define USEASMCLOCK 0
#if USE_PERFORMANCE_COUNT
LARGE_INTEGER g_Frequency;
#endif

bool drgEngine::m_CloseApp = false;
drgMessageQueue *drgEngine::m_MessageQueue = NULL;
DRG_ENGINE_PLATFORM drgEngine::m_AppPlatform = PLATFORM_PC;
char drgEngine::m_AppName[32] = "engine";
unsigned int drgEngine::m_FrameNum = 1;

float drgEngine::m_fFPS = 30.0f;
double drgEngine::m_fDeltaTime = 0.001;
double drgEngine::m_fDeltaTimeScale = 1.0f;
double drgEngine::m_fTotalTime = 0.0;
double drgEngine::m_fStartTime = 0.0;
unsigned int drgEngine::m_MinFramePeriod;
char drgEngine::m_strFrameStats[256] = "";
GUI_Manager *drgEngine::m_GuiManager = NULL;
drgMaterialManager *drgEngine::m_MaterialManager = NULL;
drgObjectList *drgEngine::m_DelayedDeleteList = NULL;
drgEngine::drgEngineMessageProcessor *drgEngine::m_MessageProcessor = NULL;

#define NUM_DELAYED_FRAMES 2
#define DRG_DEFAULT_XDPI 94
#define DRG_DEFAULT_YDPI 94
#define DRG_DEFAULT_SCREEN_DPI 96

drgEngine::drgDeleteNode::drgDeleteNode(drgObjectBase *object)
{
	assert(object); // We only delete valid pointers
	m_NumFramesRemaining = NUM_DELAYED_FRAMES;
	m_ObjectToDelete = object;
}

void setForegroundWindow(HWND hWnd);

static CHAR *drgGetLastErrorTextWin(CHAR *pBuf, ULONG bufSize)
{
	DWORD retSize;
	CHAR *pTemp = NULL;
	if (bufSize < 16)
	{
		if (bufSize > 0)
			pBuf[0] = '\0';
		return (pBuf);
	}
	retSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, GetLastError(), LANG_NEUTRAL, pTemp, 0, NULL);
	if (!retSize || pTemp == NULL)
	{
		pBuf[0] = '\0';
	}
	else
	{
		pTemp[strlen(pTemp) - 2] = '\0'; // remove cr and newline character
		sprintf(pBuf, "%0.*s (0x%x)", bufSize - 16, pTemp, GetLastError());
		LocalFree((HLOCAL)pTemp);
	}
	return (pBuf);
}

struct NativeWindowinfo
{
	// Platform specific window resources
	HDC DC; // Private GDI device context
#if USE_OPEN_GL
	HGLRC context; // Permanent rendering context
#endif
	int modeID;			   // Mode ID for fullscreen mode
	HHOOK keyboardHook;	   // Keyboard hook handle
	DWORD dwStyle;		   // Window styles used for window creation
	DWORD dwExStyle;	   // --"--
	int fullscreen;		   // Fullscreen flag
	int mouseLock;		   // Mouse-lock flag
	int oldMouseLock;	   // Mouse-lock flag
	int oldMouseLockValid; // Mouse-lock flag
	int autoPollEvents;	   // Auto polling flag
	int sysKeysDisabled;   // System keys disabled flag
	int windowNoResize;	   // Resize- and maximize gadgets disabled flag
	int refreshRate;	   // Vertical monitor refresh rate
	int opened;			   // Flag telling if window is opened or not
	int active;			   // Application active flag
	int iconified;		   // Window iconified flag
	int width;			   // Window width
	int height;			   // Window heigth
	bool isModal;		   // is this window a modal window? If so re-enable my parent when I close.
	char title[256];
	wchar_t classname[256];
	drgRenderContext *render_ctx;
	GUI_Window *win;
	NativeWindowinfo *next;
	GLFWwindow *glfw_window;
};

NativeWindowinfo *g_MainWindConfig = NULL;

HINSTANCE g_Instance; // Instance of the application
NativeWindowinfo *g_Root = NULL;
NativeWindowinfo *g_InCreate = NULL;

void WindoInfoAdd(NativeWindowinfo *win)
{
	if (g_Root == NULL)
	{
		g_Root = win;
		win->next = NULL;
	}
	else
	{
		NativeWindowinfo *prev = NULL;
		NativeWindowinfo *itr = g_Root;

		while (itr)
		{
			if (itr->next == NULL)
			{
				itr->next = win;
				win->next = NULL;
				return;
			}
			prev = itr;
			itr = itr->next;
		}
	}
}

void WindoInfoRemove(NativeWindowinfo *win)
{
	NativeWindowinfo *prev = NULL;
	NativeWindowinfo *itr = g_Root;
	while (itr)
	{
		if (itr == win)
		{
			if (prev == NULL)
				g_Root = itr->next;
			else
				prev->next = itr->next;
			return;
		}
		prev = itr;
		itr = itr->next;
	}
}

NativeWindowinfo *WindoInfoFind(GUI_Window *win)
{
	NativeWindowinfo *itr = g_Root;
	while (itr)
	{
		if (itr->win == win)
			return itr;
		itr = itr->next;
	}
	return g_InCreate;
}

NativeWindowinfo *GetNextWindowInfo(NativeWindowinfo *winInfo = NULL)
{
	if (winInfo == NULL)
		return g_Root;
	else
		return winInfo->next;
}

static void getWindowsDPI(float &xDPI, float &yDPI)
{
	int dpiX = GetDeviceCaps(g_MainWindConfig->DC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(g_MainWindConfig->DC, LOGPIXELSY);

	xDPI = (float)dpiX;
	yDPI = (float)dpiY;
}

int setMinMaxAnimations(int enable)
{
	ANIMATIONINFO AI;
	int old_enable;
	AI.cbSize = sizeof(ANIMATIONINFO);
	SystemParametersInfo(SPI_GETANIMATION, AI.cbSize, &AI, 0);
	old_enable = AI.iMinAnimate;
	if (old_enable != enable)
	{
		AI.iMinAnimate = enable;
		SystemParametersInfo(SPI_SETANIMATION, AI.cbSize, &AI, SPIF_SENDCHANGE);
	}
	return old_enable;
}

void setForegroundWindow(HWND hWnd)
{
	int try_count = 0;
	int old_animate;
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
	if (hWnd == GetForegroundWindow())
	{
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, SPIF_SENDCHANGE);
		return;
	}

	old_animate = setMinMaxAnimations(0);
	do
	{
		ShowWindow(hWnd, SW_HIDE);
		ShowWindow(hWnd, SW_SHOWMINIMIZED);
		ShowWindow(hWnd, SW_SHOWNORMAL);
		BringWindowToTop(hWnd);
		SetForegroundWindow(hWnd);
		try_count++;
	}

	while (hWnd != GetForegroundWindow() && try_count <= 3);
	(void)setMinMaxAnimations(old_animate);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, SPIF_SENDCHANGE);
}

static void getFullWindowSize(NativeWindowinfo *win_info, int clientWidth, int clientHeight, int *fullWidth, int *fullHeight)
{
	RECT rect;
	rect.left = (long)0;
	rect.right = (long)clientWidth - 1;
	rect.top = (long)0;
	rect.bottom = (long)clientHeight - 1;

	AdjustWindowRectEx(&rect, win_info->dwStyle, FALSE, win_info->dwExStyle);

	*fullWidth = rect.right - rect.left + 1;
	*fullHeight = rect.bottom - rect.top + 1;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		drgEvent::CallKeyClick(g_Root->win->GetGUID(), GLFW_KEY_W, GLFW_KEY_W, (action == GLFW_PRESS));
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			drgEngine::OnMouseDown(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_LEFT);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			drgEngine::OnMouseDown(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_RIGHT);
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			drgEngine::OnMouseDown(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_MIDDLE);
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			drgEngine::OnMouseClick(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_LEFT);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			drgEngine::OnMouseClick(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_RIGHT);
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			drgEngine::OnMouseClick(g_Root->win->GetGUID(), DRG_MOUSE_BUTTON_MIDDLE);
			break;
		default:
			break;
		}
	}
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	drgVec2 position(xpos, ypos);
	drgEngine::OnMouseMove(g_Root->win->GetGUID(), &position, &position);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	drgEngine::OnMouseWheel(g_Root->win->GetGUID(), yoffset);
}

void resize_callback(GLFWwindow *window, int width, int height)
{
	drgVec2 NewSize(width, height);
	drgEvent::CallResize(g_Root->win->GetGUID(), &NewSize);
	g_Root->width = (int)g_Root->win->WidthBox();
	g_Root->height = (int)g_Root->win->HeightBox();
}

static bool createWindow(NativeWindowinfo *win_info, drgVec2 *windowPos, unsigned int windowFlags)
{
	if (g_MainWindConfig != NULL)
	{
		return false;
	}

	// Setup surface
#ifdef __EMSCRIPTEN__
	// Create instance
	instance = wgpuCreateInstance(nullptr);
	assert(instance);

	WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
	canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector; // WGPUSType_SurfaceSourceHTMLCanvasElement;
	canvasDesc.selector = "#canvas";
	WGPUSurfaceDescriptor surfaceDesc = {};
	surfaceDesc.nextInChain = &canvasDesc.chain;
	surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
#else
#if USE_WEBGPU
	// Create instance
	drgRenderContext_WEBGPU *webgpu_context = (drgRenderContext_WEBGPU *)(win_info->render_ctx);

	WGPUInstanceDescriptor instanceDesc = {};
	webgpu_context->m_Instance = wgpuCreateInstance(&instanceDesc);
	assert(webgpu_context->m_Instance);
#endif // USE_WEBGPU

	if (!glfwInit())
	{
		drgPrintOut("Failed to initialize GLFW\r\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	win_info->glfw_window = glfwCreateWindow(win_info->width, win_info->height, win_info->title, NULL, NULL);
	if (!win_info->glfw_window)
	{
		glfwTerminate();
		return false;
	}
	if (windowPos)
	{
		glfwSetWindowPos(win_info->glfw_window, (int)windowPos->x, (int)windowPos->y);
	}

#if USE_WEBGPU
	WGPUSurfaceSourceWindowsHWND hwndDesc = {};
	hwndDesc.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
	hwndDesc.hinstance = GetModuleHandle(nullptr);
	hwndDesc.hwnd = glfwGetWin32Window(win_info->glfw_window);
	WGPUSurfaceDescriptor surfaceDesc = {};
	surfaceDesc.nextInChain = &hwndDesc.chain;
	webgpu_context->m_Surface = wgpuInstanceCreateSurface(webgpu_context->m_Instance, &surfaceDesc);
	assert(webgpu_context->m_Surface);
#endif // USE_WEBGPU
#endif // __EMSCRIPTEN__

	if (g_MainWindConfig == NULL)
	{
		g_MainWindConfig = win_info;
#if USE_WEBGPU
		webgpu_context->SetupWebGPU();
		SetContextWGPU((drgRenderContext_WEBGPU *)g_MainWindConfig->render_ctx);
#endif
	}

	// // Initialize mouse position data
	// POINT pos;
	// POINT screenPos;
	// GetCursorPos(&pos);
	// ScreenToClient(win_info->window, &screenPos);

	double mouse_xpos = 0.0;
	double mouse_ypos = 0.0;
	glfwGetCursorPos(win_info->glfw_window, &mouse_xpos, &mouse_ypos);

	drgVec2 NewMouse((float)mouse_xpos, (float)mouse_ypos);
	drgInputMouse::AddPos(&NewMouse);

	drgVec2 GlobalMouse((float)mouse_xpos, (float)mouse_ypos);
	drgInputMouse::AddGlobalPos(&GlobalMouse);

	// Set input callbacks
	glfwSetKeyCallback(win_info->glfw_window, key_callback);
	glfwSetMouseButtonCallback(win_info->glfw_window, mouse_button_callback);
	glfwSetCursorPosCallback(win_info->glfw_window, cursor_position_callback);
	glfwSetScrollCallback(win_info->glfw_window, scroll_callback);
	glfwSetWindowSizeCallback(win_info->glfw_window, resize_callback);

	if (drgEngine::GetMaterialManager() == NULL)
	{
#if USE_OPEN_GL
		glfwMakeContextCurrent(win_info->glfw_window);
		gladLoadGL();
		glfwSwapInterval(1);
#endif

		drgEngine::InitDrawing();

		// g_Shader = MakeShaderProgram(default_vertex_shader , default_fragment_shader);
		// if (g_Shader == 0u)
		//{
		//	drgPrintOut("ERROR: during creation of the shader program\n");
		//	return false;
		// }

		// uloc_project   = pglGetUniformLocation(g_Shader, "project");
		// uloc_modelview = pglGetUniformLocation(g_Shader, "modelview");

		///* Compute the projection matrix */
		// f = 1.0f / tanf(view_angle / 2.0f);

		// projection_matrix[0]  = f / aspect_ratio;
		// projection_matrix[5]  = f;
		// projection_matrix[10] = (z_far + z_near)/ (z_near - z_far);
		// projection_matrix[11] = -1.0f;
		// projection_matrix[14] = 2.0f * (z_far * z_near) / (z_near - z_far);
		// pglUniformMatrix4fv(uloc_project, 1, GL_FALSE, projection_matrix);

		///* Set the camera position */
		// modelview_matrix[12]  = -5.0f;
		// modelview_matrix[13]  = -5.0f;
		// modelview_matrix[14]  = -20.0f;
		// pglUniformMatrix4fv(uloc_modelview, 1, GL_FALSE, modelview_matrix);

		drgFont::InitDefaultFont();
	}
	// pglUseProgram(g_Shader);

	return true;
}

void drgEngine::Init()
{
	m_FrameNum = 1;
	m_fFPS = 1200.0f;
	m_fDeltaTime = 0.001;
	m_fDeltaTimeScale = 1.0f;
	m_fTotalTime = 0.0;
	m_strFrameStats[0] = '\0';

	// drgAnimation::AnimInit();

	drgDebug::Init();
	// drgDrawCommandBufferManager::Init();
	SetMaxFrameRate(m_fFPS);

	// HRESULT result = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
	// if (FAILED(result))
	// 	drgPrintOut("Error creating COM object!\r\n");

#if USE_PERFORMANCE_COUNT
	QueryPerformanceFrequency(&g_Frequency);
#endif
	m_fStartTime = (double)((GetClockCount()) / 1000.0);

	m_CloseApp = false;

	MEM_INFO_SET_NEW;
	m_GuiManager = new GUI_Manager;
	m_GuiManager->Init();

	MEM_INFO_SET_NEW;
	m_DelayedDeleteList = new drgObjectList();
	MEM_INFO_SET_NEW;
	m_MessageQueue = new drgMessageQueue;
	MEM_INFO_SET_NEW;
	m_MessageProcessor = new drgEngine::drgEngineMessageProcessor;
}

void drgEngine::CleanUp()
{
	// Must be the last thing released (at least last thing after anything that holds a drgObject)
	ReleaseDelayedDeleteList();

	glfwTerminate();
	// CoUninitialize();
}

void drgEngine::InitDrawing()
{
	MEM_INFO_SET_NEW;
	m_MaterialManager = new drgMaterialManager;
	m_MaterialManager->Init("material_pool.json");
}

bool drgEngine::IsMultiWindow()
{
	return true;
}

void drgEngine::Close()
{
	m_CloseApp = true;
	if (m_GuiManager)
	{
		delete m_GuiManager;
		m_GuiManager = NULL;
	}
}

void drgEngine::Update()
{
	UpdateDelayedDeleteList();
	UpdateStats();

	glfwPollEvents();

	drgEngine::ProcessMessages();
	if (m_GuiManager)
	{
		m_GuiManager->Update(GetDeltaTime());
	}

	GUI_Window *win = m_GuiManager->GetBaseWindow();
	NativeWindowinfo *wndconfig = (NativeWindowinfo *)win->m_NativeData;
	if (glfwWindowShouldClose(wndconfig->glfw_window))
	{
		Close();
	}
}

void drgEngine::Draw()
{
	if (m_GuiManager)
	{
		m_GuiManager->Draw();
	}
}

bool drgEngine::IsClosing()
{
	return m_CloseApp;
}

void drgEngine::LoadingScreenStart()
{
}

void drgEngine::LoadingScreenEnd()
{
}

bool drgEngine::NativeWindowCreate(GUI_Window *win, string16 *title, drgVec2 *pos, unsigned int windowFlags)
{
	static unsigned int classcount = 0;

	if (g_Instance == NULL)
	{
		g_Instance = GetModuleHandle(NULL);
	}

	NativeWindowinfo *wndconfig = (NativeWindowinfo *)drgMemAllocZero(sizeof(NativeWindowinfo));
	WindoInfoAdd(wndconfig);
	g_InCreate = wndconfig;

	win->SetFlagsWin(windowFlags);
	m_GuiManager->AddWindow(win);

	win->m_NativeData = wndconfig;
	wndconfig->win = win;
	wndconfig->refreshRate = 60;
	wndconfig->windowNoResize = (windowFlags & DRG_WINDOW_NORESIZE);
	wndconfig->width = (int)win->WidthBox();
	wndconfig->height = (int)win->HeightBox();
	MEM_INFO_SET_NEW;
#if USE_OPEN_GL
	wndconfig->render_ctx = new drgRenderContext_OGL();
#elif USE_WEBGPU
	wndconfig->render_ctx = new drgRenderContext_WEBGPU();
#else
	wndconfig->render_ctx = new drgRenderContext();
#endif

	if (wndconfig->fullscreen)
	{
		// TODO:
	}

	string8 win_title(*title);
	drgString::Copy(wndconfig->title, win_title.c_str());

	if (!createWindow(wndconfig, pos, windowFlags))
	{
		drgPrintError("Failed to create window\n");
		g_InCreate = NULL;
		return false;
	}

	// if (wndconfig->fullscreen)
	// 	SetWindowPos(wndconfig->window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// setForegroundWindow(wndconfig->window);
	// SetFocus(wndconfig->window);

	MEM_INFO_SET_NEW;
	win->m_DrawContext = new GUI_Drawing();
	wndconfig->render_ctx->Init();
	wndconfig->render_ctx->SetDrawSize((float)wndconfig->width, (float)wndconfig->height);
	win->m_DrawContext->Init(wndconfig->render_ctx);

	// Get the DPI
	float xDPI = 0.0f;
	float yDPI = 0.0f;
	getWindowsDPI(xDPI, yDPI);
	// win->m_DrawContext->GetRenderContext()->SetupDPI( xDPI, yDPI, DRG_DEFAULT_XDPI, DRG_DEFAULT_YDPI );

	// Remove borders here.
	if (windowFlags & DRG_WINDOW_NOBORDERS)
	{
		drgEngine::NativeWindowDeactivateBorder(wndconfig->win);
	}

	// // We have to add the thickframe style after the window has been created. I don't know why.
	// if( windowFlags & DRG_WINDOW_NOTITLE )
	// {
	// 	SetWindowLong( wndconfig->window, GWL_STYLE, WS_THICKFRAME );
	// 	SetWindowLong( wndconfig->window, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	// 	ShowWindow( wndconfig->window, SW_SHOW );

	// 	updateWindowSize( wndconfig->window );
	// }

	g_InCreate = NULL;
	return true;
}

bool drgEngine::NativeContextCreate(GUI_Window *win)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;
	// if(!createContext(wndconfig))
	//	return false;

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	// int iFormat = ChoosePixelFormat( wndconfig->DC, &pfd );
	// SetPixelFormat( wndconfig->DC, iFormat, &pfd );

	return true;
}

void *drgEngine::NativeContextSetMain()
{
	// #if USE_OPEN_GL
	// 	if (g_MainWindConfig != NULL)
	// 		wglMakeCurrent(g_MainWindConfig->DC, g_MainWindConfig->context);
	// #endif
	return g_MainWindConfig->render_ctx;
}

void drgEngine::NativeWindowCleaup(GUI_Window *win)
{
	NativeWindowinfo *wndconfig = (NativeWindowinfo *)win->m_NativeData;

	// #if USE_OPEN_GL
	// 	wglMakeCurrent(wndconfig->DC, wndconfig->context);
	// 	delete wndconfig->render_ctx;
	// 	glFinish();
	// 	wglMakeCurrent(NULL, NULL);
	// #endif

	if (wndconfig->isModal)
	{
		// Remember to re-enable our parent
		GUI_Window *parentWin = win->GetParentWindow();
		if (parentWin)
		{
			NativeWindowinfo *parentWinConfig = WindoInfoFind(parentWin);
			if (parentWinConfig)
			{
				// EnableWindow(parentWinConfig->window, TRUE);
			}
		}
	}

	glfwDestroyWindow(wndconfig->glfw_window);
	// ReleaseDC(wndconfig->window, wndconfig->DC);
	// DestroyWindow(wndconfig->window);
	// BOOL unreg_ret = UnregisterClass(wndconfig->classname, NULL);
	WindoInfoRemove(wndconfig);
	if (g_MainWindConfig == wndconfig)
		g_MainWindConfig = NULL;
	drgMemFree(win->m_NativeData);
}

void drgEngine::NativeWindowBegin(GUI_Window *win)
{
	drgEngine::NativeContextSetMain();

	// pglUseProgram(g_Shader);
	// drgMaterialManager::FlushCache();

	NativeWindowinfo *wndconfig = (NativeWindowinfo *)win->m_NativeData;
	// #if USE_OPEN_GL
	// 	wglMakeCurrent(wndconfig->DC, wndconfig->context);
	// 	assert(GetLastError() == 0); // TODO: This needs to become a command.
	// #endif
	// win->GetDrawContext()->GetCommandBuffer()->MakeCurrent( wndconfig->context, wndconfig->DC );
	wndconfig->render_ctx->Begin();
}

void drgEngine::NativeWindowEnd(GUI_Window *win)
{
	NativeWindowinfo *wndconfig = (NativeWindowinfo *)win->m_NativeData;
	wndconfig->render_ctx->End();
	glfwSwapBuffers(wndconfig->glfw_window);

#if USE_OPEN_GL
	// SwapBuffers(wndconfig->DC); // TODO: This needs to become a command.
#endif
	wndconfig->render_ctx->Begin();
}

void drgEngine::NativeWindowSetPosition(GUI_Window *win, float x, float y)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;
	// BOOL result = SetWindowPos( wndconfig->window, HWND_TOP, (int)x, (int)y, 0, 0, SWP_NOSIZE );
	// assert( result == TRUE && "Failed to set the native window's position" );
}

drgVec2 drgEngine::NativeWindowGetPosition(GUI_Window *win)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;

	// RECT rect;
	// GetWindowRect( wndconfig->window, &rect );

	// RECT clientRect;
	// GetClientRect( wndconfig->window, &clientRect );

	// LONG xOffset = (rect.right - rect.left) - clientRect.right;
	// LONG yOffset = (rect.bottom - rect.top) - clientRect.bottom;

	// drgVec2 ret( (float)(rect.left + xOffset), (float)(rect.top + yOffset));
	drgVec2 ret;
	return ret;
}

void drgEngine::NativeMouseCapture(GUI_Window *win)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;
	// SetCapture( wndconfig->window );
}

void drgEngine::NativeWindowActivateBorder(GUI_Window *win)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;

	// LONG style = GetWindowLong( wndconfig->window, GWL_STYLE );
	// style |= (WS_CAPTION | WS_THICKFRAME);
	// SetWindowLong( wndconfig->window, GWL_STYLE, style );

	// LONG exStyle = GetWindowLong( wndconfig->window, GWL_EXSTYLE );
	// exStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	// SetWindowLong( wndconfig->window, GWL_EXSTYLE, exStyle );

	// ShowWindow( wndconfig->window, SW_SHOWNORMAL );
	// updateWindowSize( wndconfig->window );
}

void drgEngine::NativeWindowDeactivateBorder(GUI_Window *win)
{
	// NativeWindowinfo* wndconfig = (NativeWindowinfo*)win->m_NativeData;

	// LONG style = GetWindowLong( wndconfig->window, GWL_STYLE );
	// style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	// SetWindowLong( wndconfig->window, GWL_STYLE, style );

	// LONG exStyle = GetWindowLong( wndconfig->window, GWL_EXSTYLE );
	// exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	// SetWindowLong( wndconfig->window, GWL_EXSTYLE, exStyle );

	// ShowWindow( wndconfig->window, SW_SHOWNORMAL );
	// //updateWindowSize( wndconfig->window );
}

void drgEngine::UpdateStats()
{
	DRG_PROFILE_FUNK(UpdateStats, 0xFFFFFF00);

	// Keep track of the frame count
	static drgUInt64 fLastTime = 0;
	static drgUInt64 fTime;
	drgUInt64 fTimeChange = 0;
	++m_FrameNum;
	fTime = GetClockCount();
	fTimeChange = (fTime - fLastTime);
	while (fTimeChange < m_MinFramePeriod)
	{
		Sleep(m_MinFramePeriod - (unsigned int)fTimeChange);
		// drgThread::SleepCurrent( m_MinFramePeriod-(unsigned int)fTimeChange );
		fTime = GetClockCount();
		fTimeChange = (fTime - fLastTime);
	};
	m_fDeltaTime = ((double)(((double)(fTime - fLastTime)) / 1000.0)) * m_fDeltaTimeScale;
	fLastTime = fTime;
	m_fTotalTime = (double)(((double)(fTime)) / 1000.0);
	if (m_fDeltaTime > 0.0)
	{
		m_fFPS = ((float)(1.00 / m_fDeltaTime) * 0.1f) + (m_fFPS * 0.9f);
	}

	// if(!m_SwapLock)
	//{
	//	g_CountAvgFps--;
	//	if(g_CountAvgFps<=0)
	//	{
	//		g_AvgFps[g_CurAvgFps]=m_fFPS;
	//		g_CurAvgFps++;
	//		if(g_CurAvgFps>9)
	//			g_CurAvgFps=0;
	//		g_CountAvgFps=30;
	//		float avgfps = (g_AvgFps[0]+g_AvgFps[1]+g_AvgFps[2]+g_AvgFps[3]+g_AvgFps[4]+g_AvgFps[5]+g_AvgFps[6]+g_AvgFps[7]+g_AvgFps[8]+g_AvgFps[9])*0.1f;
	//		m_CurrentPerformance =(unsigned int)DRG_CLAMP(((avgfps/50.0f)*100.0f), 0.0f, (float)m_MaxPerformance);
	//		if(m_FrameNum>100)
	//			m_MaxPerformance = (unsigned int)(((((float)m_MaxPerformance)*3.0f)+((float)m_CurrentPerformance))*0.25f);
	//		m_CurrentPerformance = DRG_MAX(m_CurrentPerformance, m_MinPerformance);
	//	}
	// }

	if (m_fDeltaTime > 0.2)
		m_fDeltaTime = 0.01;

	float ShaderTime = (float)(m_fTotalTime);
	float timeout[4] = {(ShaderTime * 50.0f), ShaderTime, (ShaderTime * 100.0f), (ShaderTime * 0.01f)};

#ifndef RETAIL_BUILD
	// Update the scene stats once per second
	double fFrameDelta;
	static drgUInt64 fLastTimeStat = 0;
	static unsigned int iLastFrame = 0;
	if ((fTime - fLastTimeStat) > 1000.0f)
	{
		fFrameDelta = (float)(m_FrameNum - iLastFrame);
		fLastTimeStat = fTime;
		iLastFrame = m_FrameNum;

		drgString::PrintFormated(m_strFrameStats, "fps = %.02f %u", m_fFPS, (unsigned int)fTimeChange, m_FrameNum);
	}
#endif
}

drgInt64 drgEngine::GetClockCount()
{
#if USE_PERFORMANCE_COUNT
	LARGE_INTEGER liCurrent;
	QueryPerformanceCounter(&liCurrent);
	unsigned int ret = (unsigned int)((liCurrent.QuadPart * 1000) / g_Frequency.QuadPart);
#else

#if USEASMCLOCK
	__int64 ret;
	__asm
		{
        rdtsc // Resad the RDTSC Timer
        mov    dword ptr[ret], eax // Store the value in EAX and EDX Registers
        mov    dword ptr[ret+4], edx
		}
#elif 0
	DWORD dwCurrent = timeGetTime();
	unsigned int ret = (unsigned int)(dwCurrent);
#else
	DWORD dwCurrent = GetTickCount();
	unsigned int ret = (unsigned int)(dwCurrent);
#endif

#endif
	return ret;
}

void drgEngine::DelayDeleteObject(drgObjectBase *object)
{
	if (m_DelayedDeleteList != NULL)
	{
		MEM_INFO_SET_NEW;
		drgDeleteNode *node = new drgDeleteNode(object);
		m_DelayedDeleteList->AddTail(node);
	}
	else
	{
		SAFE_DELETE(object);
	}
}

void drgEngine::UpdateDelayedDeleteList()
{
	if (m_DelayedDeleteList == NULL)
		return;

	drgDeleteNode *delnode;
	drgDeleteNode *node = (drgDeleteNode *)m_DelayedDeleteList->GetLast();
	while (node)
	{
		if (--node->m_NumFramesRemaining <= 0)
		{
			delnode = node;
			node = (drgDeleteNode *)m_DelayedDeleteList->GetPrev(node);
			m_DelayedDeleteList->Remove(delnode);
			delete delnode->m_ObjectToDelete;
			delete delnode;
		}
		else
		{
			node = (drgDeleteNode *)m_DelayedDeleteList->GetPrev(node);
		}
	}
}

void drgEngine::ReleaseDelayedDeleteList()
{
	if (m_DelayedDeleteList == NULL)
		return;

	drgDeleteNode *node = (drgDeleteNode *)m_DelayedDeleteList->GetLast();
	while (node)
	{
		drgDeleteNode *temp = (drgDeleteNode *)m_DelayedDeleteList->GetPrev(node);
		m_DelayedDeleteList->Remove(node);
		delete node->m_ObjectToDelete;
		delete node;

		node = temp;
	}

	SAFE_DELETE(m_DelayedDeleteList);
}

bool drgEngine::SetClipboardText(string16 *text)
{
	BOOL ok = FALSE;
	if (OpenClipboard(NULL))
	{
		// the text should be placed in "global" memory
		HGLOBAL hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, (text->length() + 1) * 2);
		WCHAR *ptxt = (WCHAR *)GlobalLock(hMem);
		lstrcpyW(ptxt, (WCHAR *)text->str());
		GlobalUnlock(hMem);
		// set data in clipboard; we are no longer responsible for hMem
		ok = !!::SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard(); // relinquish it for other windows
	}
	return ok != 0;
}

int drgEngine::GetClipboardText(string16 *text)
{
	int i = 0;
	*text = "";
	if (OpenClipboard(NULL))
	{
		HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
		if (hMem)
		{ // format is available, extract text
			short *ptxt = (short *)GlobalLock(hMem);
			for (; ++i < *ptxt;) // copy as much as will fit
				(*text) += *ptxt++;
			GlobalUnlock(hMem); // we don't free it; owned by clipboard
		}
		CloseClipboard();
	}
	return i; // 0 indicates failure
}

void drgEngine::GetDeviceID(char device_st[64])
{
	drgString::Copy(device_st, "DRG");
	// drgNetManager::GetMAC(device_st);
	drgString::Concatenate(device_st, "04567");
}

void drgEngine::OpenExternalUrl(char *url)
{
	HKEY key;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\OpenWithList"), &key) == ERROR_SUCCESS)
	{
		char value_first[256];
		DWORD value_length_first = 256;
		DWORD dwType = 0;
		if (RegQueryValueExA(key, "MRUList", NULL, &dwType, (LPBYTE)&value_first, &value_length_first) == ERROR_SUCCESS)
		{
			char value[256];
			DWORD value_length = 256;
			if (RegQueryValueExA(key, value_first, NULL, &dwType, (LPBYTE)&value, &value_length) == ERROR_SUCCESS)
			{
				ShellExecuteA(NULL, "open", value, url, NULL, SW_SHOWDEFAULT);
			}
		}
		RegCloseKey(key);
	}
}

void drgEngine::OpenExternalEmail(char *addr, char *subject, char *text)
{
}

void drgEngine::OpenAppRating()
{
}

bool drgEngine::CanShowAd(DRG_AD_TYPE type)
{
	return true;
}

bool drgEngine::ShowAd(DRG_AD_TYPE type)
{
	bool retval = false;
	HKEY key;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\OpenWithList"), &key) == ERROR_SUCCESS)
	{
		char value_first[256];
		DWORD value_length_first = 256;
		DWORD dwType = 0;
		if (RegQueryValueExA(key, "MRUList", NULL, &dwType, (LPBYTE)&value_first, &value_length_first) == ERROR_SUCCESS)
		{
			char value[256];
			DWORD value_length = 256;
			if (RegQueryValueExA(key, value_first, NULL, &dwType, (LPBYTE)&value, &value_length) == ERROR_SUCCESS)
			{
				ShellExecuteA(NULL, "open", value, "https://www.youtube.com/watch?v=dQw4w9WgXcQ", NULL, SW_SHOWDEFAULT);
				drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONAD_DONE, NULL, false);
				drgEngine::AddEngineMessage(record);
				retval = true;
			}
		}
		RegCloseKey(key);
	}
	return retval;
}

long long drgEngine::ShowThirdPartyLogin(DRG_THIRD_PARTY_LOGIN_TYPE type)
{
	char *returned_id = (char *)malloc(256);
	strcpy(returned_id, "10152155717566969/Dylan");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONTHIRDPARTYLOGIN_DONE, returned_id, true);
	drgEngine::AddEngineMessage(record);
	return 0;
}

bool drgEngine::GetThirdPartyLoginReady(DRG_THIRD_PARTY_LOGIN_TYPE type)
{
	return true;
}

long long drgEngine::GetThirdPartyLoginId(DRG_THIRD_PARTY_LOGIN_TYPE type)
{
	return 50;
}

void drgEngine::GetMerchantSkuData(DRG_MERCHANT_TYPE type, char *skulist)
{
	// const char* retbuff = "cred_test_10000~$0.99~10,000 credits (Slot Fantasy -Voted Best Slots)~Buy 10,000 credits instantly!^cred_test_25000~$1.99~25,000 credits (25% FREE!) (Slot Fantasy -Voted Best Slots)~Buy 20,000 credits and get 5,000 bonus credits free!!!^feat_test_double_win_3day~$0.99~Double All Wins (3 days) (Slot Fantasy -Voted Best Slots)~Double all of your wins and jackpots for 3 days!^feat_test_double_wins~$9.99~Double All Wins (Permanent) (Slot Fantasy -Voted Best Slots)~Double all of your jackpots and bonuses forever!!!^cred_test_10000~$0.99~10,000 credits (Slot Fantasy -Voted Best Slots)~Buy 10,000 credits instantly!^cred_test_25000~$1.99~25,000 credits (25% FREE!) (Slot Fantasy -Voted Best Slots)~Buy 20,000 credits and get 5,000 bonus credits free!!!^feat_test_double_win_3day~$0.99~Double All Wins (3 days) (Slot Fantasy -Voted Best Slots)~Double all of your wins and jackpots for 3 days!^feat_test_double_wins~$9.99~Double All Wins (Permanent) (Slot Fantasy -Voted Best Slots)~Double all of your jackpots and bonuses forever!!!^cred_test_10000~$0.99~10,000 credits (Slot Fantasy -Voted Best Slots)~Buy 10,000 credits instantly!^cred_test_25000~$1.99~25,000 credits (25% FREE!) (Slot Fantasy -Voted Best Slots)~Buy 20,000 credits and get 5,000 bonus credits free!!!^feat_test_double_win_3day~$0.99~Double All Wins (3 days) (Slot Fantasy -Voted Best Slots)~Double all of your wins and jackpots for 3 days!^feat_test_double_wins~$9.99~Double All Wins (Permanent) (Slot Fantasy -Voted Best Slots)~Double all of your jackpots and bonuses forever!!!^cred_test_10000~$0.99~10,000 credits (Slot Fantasy -Voted Best Slots)~Buy 10,000 credits instantly!^cred_test_25000~$1.99~25,000 credits (25% FREE!) (Slot Fantasy -Voted Best Slots)~Buy 20,000 credits and get 5,000 bonus credits free!!!^feat_test_double_win_3day~$0.99~Double All Wins (3 days) (Slot Fantasy -Voted Best Slots)~Double all of your wins and jackpots for 3 days!^feat_test_double_wins~$9.99~Double All Wins (Permanent) (Slot Fantasy -Voted Best Slots)~Double all of your jackpots and bonuses forever!!!^cred_test_10000~$0.99~10,000 credits (Slot Fantasy -Voted Best Slots)~Buy 10,000 credits instantly!^cred_test_25000~$1.99~25,000 credits (25% FREE!) (Slot Fantasy -Voted Best Slots)~Buy 20,000 credits and get 5,000 bonus credits free!!!^feat_test_double_win_3day~$0.99~Double All Wins (3 days) (Slot Fantasy -Voted Best Slots)~Double all of your wins and jackpots for 3 days!^feat_test_double_wins~$9.99~Double All Wins (Permanent) (Slot Fantasy -Voted Best Slots)~Double all of your jackpots and bonuses forever!!!";
	// drgMessageRecord* record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSKUDETAILSRECEIVED, retbuff, false);
	// drgEngine::AddEngineMessage(record);
}

void drgEngine::PurchaseMerchantSku(DRG_MERCHANT_TYPE type, char *sku)
{
	return;
}

void drgEngine::GetABTestingProjects(char *projects)
{
	MEM_INFO_SET_NEW;
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONABTESTINGVALUESRECEIVED, NULL, false);
	drgEngine::AddEngineMessage(record);
}

void drgEngine::GetABTestingProjectValues(char *project, char *values, char *outvalues)
{
	strcpy(outvalues, values);
}

void drgEngine::TriggerInsightEvent(char *eventname)
{
}

void drgEngine::TriggerInsightEvent(char *eventname, char *attributes, char *metrics)
{
}

void drgEngine::SubmitQueuedInsightEvents()
{
}

void drgEngine::GetPushNotificationToken(char *token)
{
	strcpy(token, "");
}

void drgEngine::drgEngineMessageProcessor::ProcessMessage(drgMessageRecord *record)
{
	if (record == NULL)
		return;

	AppInterface::ProcessMessage(record->GetDataType(), (char *)record->GetData());
}

void drgEngine::ProcessMessages()
{
	if (m_MessageQueue)
	{
		if (m_MessageProcessor == NULL)
		{
			drgPrintError("Can't process messages without a message processor");
			return;
		}

		m_MessageQueue->ProcessRecords(m_MessageProcessor);
		m_MessageQueue->ClearProcessedRecords();
	}
}

void drgEngine::GetScreenSize(int *width, int *height)
{
	if (m_GuiManager)
	{
		GUI_Window *window = m_GuiManager->GetBaseWindow();
		if (window)
		{
			*width = (int)window->WidthView();
			*height = (int)window->HeightView();
			return;
		}
	}

	*width = *height = 0;
}

void drgEngine::EnableScreenTimeout(bool enable)
{
}

void drgEngine::OnMouseDown(unsigned int winGUID, int buttonID)
{
	drgInputMouse::SetButtonDown(buttonID);
	drgEvent::CallMouseDown(winGUID, buttonID);
}

void drgEngine::OnMouseClick(unsigned int winGUID, int buttonID)
{
#if GAFFE_SUSPEND_RESUME
	static bool suspended = false;
	if (drgInputMouse::IsButtonDown(DRG_MOUSE_BUTTON_LEFT) && drgInputMouse::IsButtonDown(DRG_MOUSE_BUTTON_RIGHT))
	{
		if (suspended == false)
		{
			drgEngine::ReleaseResources();
			suspended = true;
		}
		else
		{
			drgEngine::RecreateResources();
			suspended = false;
		}
	}
#endif

	drgInputMouse::SetButtonUp(buttonID);
	drgEvent::CallMouseClick(winGUID, buttonID);
	drgEvent::CallMouseUp(winGUID, buttonID); // This event can't be eaten, it allows all object to reset state if necessary
}

void drgEngine::OnMouseMove(unsigned int winGUID, drgVec2 *localPos, drgVec2 *globalPos)
{
	// Add the global position.
	if (globalPos)
	{
		drgInputMouse::AddGlobalPos(globalPos);
	}

	// Add the local position and dispatch the event.
	if (drgInputMouse::AddPos(localPos))
	{
		drgEvent::CallMouseMove(winGUID, drgInputMouse::GetPos(), drgInputMouse::GetPosMove());
		// if( drgEngine::GetGuiManager()->HasDragWidget() ) {
		// 	drgEvent::CallMouseDrag( winGUID, drgInputMouse::GetPos(), drgInputMouse::GetPosMove() );
		// }
	}
}

void drgEngine::OnMouseWheel(unsigned int winGUID, float delta)
{
	drgInputMouse::AddWheel(delta);
	drgEvent::CallMouseWheel(winGUID, drgInputMouse::GetWheelPos(), delta);
}

void drgEngine::OnPointerDown(unsigned int winGUID, unsigned int pointerIndex, float pressure)
{
	// Doesn't happen on windows
}

void drgEngine::OnPointerClick(unsigned int winGUID, unsigned int pointerIndex)
{
	// Doesn't happen on windows
}

void drgEngine::OnPointerMove(unsigned int winGUID, unsigned int pointerIndex, drgVec2 *localPosition, drgVec2 *globalPosition)
{
	// Doesn't happen on windows
}

void drgEngine::ReleaseResources()
{
	drgPrintOut("************ RELEASING RESOURCES: TEXTURES ****************\n");
	drgTexture::ReleaseAllGPUResources();

	drgPrintOut("************ RELEASING RESOURCES: SHADERS ****************\n");
	drgMaterialManager::ReleaseGPUResources();

	drgPrintOut("************ RELEASING RESOURCES: CONTEXT ****************\n");
}

void drgEngine::RecreateResources()
{
	drgPrintOut("************ RECREATING RESOURCES: TEXTURES ****************\n");
	drgTexture::RecreateAllGPUResources();

	drgPrintOut("************ RECREATING RESOURCES: SHADERS *****************\n");
	drgMaterialManager::RecreateGPUResources();

	// drgPrintOut( "************ RECREATING RESOURCES: RENDER CACHE RESET ****************\n" );
	// drgDrawCommandBufferManager::ResetAllRenderCaches();
}

#endif // _WINDOWS

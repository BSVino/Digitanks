#include "digitankswindow.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <assert.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>
#include <strutils.h>

#include <mtrand.h>
#include <configfile.h>
#include <platform.h>
#include <network/network.h>
#include <sound/sound.h>
#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "hud.h"
#include "instructor.h"
#include "game/camera.h"
#include "shaders/shaders.h"
#include "menu.h"
#include "ui.h"
#include "renderer/renderer.h"
#include "register.h"

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

ConfigFile c( GetAppDataDirectory(L"Digitanks", L"options.cfg") );

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
{
	s_pDigitanksWindow = this;

	m_pGameServer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	srand((unsigned int)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_bBoxSelect = false;
	m_bCheatsOn = false;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;

	glfwInit();

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (c.isFileValid())
	{
		m_iWindowWidth = c.read<int>("width", 1024);
		m_iWindowHeight = c.read<int>("height", 768);

		m_bFullscreen = m_bCfgFullscreen = !c.read<bool>("windowed", false);
		m_bConstrainMouse = c.read<bool>("constrainmouse", true);

		SetSoundVolume(c.read<float>("soundvolume", 0.8f));
		SetMusicVolume(c.read<float>("musicvolume", 0.8f));
	}
	else
	{
		m_iWindowWidth = iScreenWidth*2/3;
		m_iWindowHeight = iScreenHeight*2/3;

#ifdef _DEBUG
		m_bFullscreen = false;
#else
		m_bFullscreen = true;
#endif
		m_bCfgFullscreen = true;
		m_bConstrainMouse = true;

		SetSoundVolume(0.8f);
		SetMusicVolume(0.8f);
	}

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

	if (HasCommandLineSwitch("--fullscreen"))
		m_bFullscreen = true;

	if (HasCommandLineSwitch("--windowed"))
		m_bFullscreen = false;

	glfwEnable( GLFW_MOUSE_CURSOR );

	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	if (!glfwOpenWindow(m_iWindowWidth, m_iWindowHeight, 0, 0, 0, 0, 16, 0, m_bFullscreen?GLFW_FULLSCREEN:GLFW_WINDOW))
	{
		glfwTerminate();
		return;
	}

	glfwSetWindowTitle( "Digitanks!" );

	int iWindowX = (int)(iScreenWidth/2-m_iWindowWidth/2);
	int iWindowY = (int)(iScreenHeight/2-m_iWindowHeight/2);
	glfwSetWindowPos(iWindowX, iWindowY);

	glfwSetWindowSizeCallback(&CDigitanksWindow::WindowResizeCallback);
	glfwSetKeyCallback(&CDigitanksWindow::KeyEventCallback);
	glfwSetCharCallback(&CDigitanksWindow::CharEventCallback);
	glfwSetMousePosCallback(&CDigitanksWindow::MouseMotionCallback);
	glfwSetMouseButtonCallback(&CDigitanksWindow::MouseInputCallback);
	glfwSetMouseWheelCallback(&CDigitanksWindow::MouseWheelCallback);
	glfwSwapInterval( 1 );
	glfwSetTime( 0.0 );
	glfwEnable( GLFW_MOUSE_CURSOR );

	DumpGLInfo();

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		ShowMessage(L"Looks like your video card doesn't support the features needed by Digitanks! We're terribly sorry.");
		exit(0);
	}

	if (strstr((const char*)glGetString(GL_VENDOR), "Intel") > 0 || strstr((const char*)glGetString(GL_VENDOR), "INTEL") > 0)
		ShowMessage(L"You are running an Intel graphics card. These cards are unsupported and may crash or display Digitanks incorrectly. More recent models may work. You're welcome to try but you really need to get an ATI or NVidia card if you want to play.");

	ilInit();

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	m_iLoading = CRenderer::LoadTextureIntoGL(L"textures/loading.png");
	RenderLoading();

	CShaderLibrary::CompileShaders();

	InitUI();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0);

	CNetwork::Initialize();

	// Save out the configuration file now that we know this config loads properly.
	SetConfigWindowDimensions(m_iWindowWidth, m_iWindowHeight);
	SaveConfig();

	ReadProductCode();
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;
	delete m_pMainMenu;

	DestroyGame();

	glfwTerminate();
}

#define MAKE_PARAMETER(name) \
{ #name, name } \

void CDigitanksWindow::DumpGLInfo()
{
	glewInit();

	std::ifstream i(GetAppDataDirectory(L"Digitanks", L"glinfo.txt"));
	if (i)
		return;
	i.close();

	std::ofstream o(GetAppDataDirectory(L"Digitanks", L"glinfo.txt"));
	if (!o || !o.is_open())
		return;

	o << "Vendor: " << (char*)glGetString(GL_VENDOR) << ENDL;
	o << "Renderer: " << (char*)glGetString(GL_RENDERER) << ENDL;
	o << "Version: " << (char*)glGetString(GL_VERSION) << ENDL;

	char* pszShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (pszShadingLanguageVersion)
		o << "Shading Language Version: " << pszShadingLanguageVersion << ENDL;

	std::string sExtensions = (char*)glGetString(GL_EXTENSIONS);
	std::vector<std::string> asExtensions;
	strtok(sExtensions, asExtensions);
	o << "Extensions:" << ENDL;
	for (size_t i = 0; i < asExtensions.size(); i++)
		o << "\t" << asExtensions[i] << ENDL;

	typedef struct
	{
		char* pszName;
		int iParameter;
	} GLParameter;

	GLParameter aParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_ATTRIB_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_CLIP_PLANES),
		MAKE_PARAMETER(GL_MAX_LIGHTS),
		MAKE_PARAMETER(GL_MAX_COLOR_MATRIX_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_MODELVIEW_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_PROJECTION_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_TEXTURE_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_3D_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB),
		MAKE_PARAMETER(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_VERTICES),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_INDICES),
		MAKE_PARAMETER(GL_MAX_EVAL_ORDER),
		MAKE_PARAMETER(GL_MAX_LIST_NESTING),
		MAKE_PARAMETER(GL_MAX_NAME_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_PIXEL_MAP_TABLE),
		MAKE_PARAMETER(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_UNITS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_LOD_BIAS_EXT),
		MAKE_PARAMETER(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT),
		MAKE_PARAMETER(GL_MAX_DRAW_BUFFERS_ARB),

		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_VARYING_FLOATS_ARB),
		MAKE_PARAMETER(GL_MAX_VERTEX_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_COORDS_ARB),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_COORDS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_IMAGE_UNITS_ARB),
	};

	// Clear it
	glGetError();

	o << ENDL;

	for (size_t i = 0; i < sizeof(aParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetIntegerv(aParameters[i].iParameter, &iValue);

		if (glGetError() != GL_NO_ERROR)
			continue;

		o << aParameters[i].pszName << ": " << iValue << ENDL;
	}

	GLParameter aProgramParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_PROGRAM_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEMPORARIES_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ENV_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB),
	};

	o << ENDL;
	o << "Vertex programs:" << ENDL;

	for (size_t i = 0; i < sizeof(aProgramParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, aProgramParameters[i].iParameter, &iValue);

		if (glGetError() == GL_NO_ERROR)
			o << aProgramParameters[i].pszName << ": " << iValue << ENDL;
	}

	o << ENDL;
	o << "Fragment programs:" << ENDL;

	for (size_t i = 0; i < sizeof(aProgramParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, aProgramParameters[i].iParameter, &iValue);

		if (glGetError() == GL_NO_ERROR)
			o << aProgramParameters[i].pszName << ": " << iValue << ENDL;
	}
}

void CDigitanksWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, m_iWindowWidth, m_iWindowHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glgui::CRootPanel::PaintTexture(m_iLoading, m_iWindowWidth/2 - 150, m_iWindowHeight/2 - 150, 300, 300);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	glfwSwapBuffers();
}

void CDigitanksWindow::CreateGame(gametype_t eGameType)
{
	RenderLoading();

	if (eGameType != GAMETYPE_MENU)
		CSoundLibrary::StopMusic();

	if (eGameType == GAMETYPE_MENU)
	{
		if (!CSoundLibrary::IsMusicPlaying() && !HasCommandLineSwitch("--no-music"))
			CSoundLibrary::PlayMusic("sound/assemble-for-victory.ogg");
	}
	else if (!HasCommandLineSwitch("--no-music"))
		CSoundLibrary::PlayMusic("sound/network-rise-network-fall.ogg", true);

	mtsrand((size_t)time(NULL));

	std::string sHost;
	sHost.assign(m_sConnectHost.begin(), m_sConnectHost.end());
	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (GameServer())
	{
		GameServer()->SetConnectHost(sHost);
		GameServer()->SetServerType(m_eServerType);
		if (eGameType == GAMETYPE_MENU)
			GameServer()->SetServerType(SERVER_LOCAL);
		GameServer()->SetServerPort(iPort);
		GameServer()->Initialize();
	}

	if (!m_pGameServer)
	{
		m_pHUD = new CHUD();

		m_pGameServer = new CGameServer();

		GameServer()->SetConnectHost(sHost);
		GameServer()->SetServerType(m_eServerType);
		if (eGameType == GAMETYPE_MENU)
			GameServer()->SetServerType(SERVER_LOCAL);
		GameServer()->SetServerPort(iPort);
		GameServer()->Initialize();

		CNetwork::SetCallbacks(m_pGameServer, CGameServer::ClientConnectCallback, CGameServer::ClientDisconnectCallback);

		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		if (!m_pInstructor)
			m_pInstructor = new CInstructor();
	}

	if (CNetwork::IsHost() && DigitanksGame())
	{
		DigitanksGame()->SetPlayers(m_iPlayers);
		DigitanksGame()->SetTanks(m_iTanks);
		DigitanksGame()->SetupGame(eGameType);
	}

	glgui::CRootPanel::Get()->Layout();

	m_pMainMenu->SetVisible(eGameType == GAMETYPE_MENU);
}

void CDigitanksWindow::DestroyGame()
{
	RenderLoading();

	CNetwork::Disconnect();

	if (m_pGameServer)
		delete m_pGameServer;

	if (m_pHUD)
	{
		glgui::CRootPanel::Get()->RemoveControl(m_pHUD);
		delete m_pHUD;
	}

	if (m_pInstructor)
		delete m_pInstructor;

	m_pGameServer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	CSoundLibrary::StopMusic();
}

void CDigitanksWindow::Run()
{
	CreateGame(GAMETYPE_MENU);

	if (!IsRegistered())
	{
		m_pMainMenu->SetVisible(false);
		m_pPurchase->OpeningApplication();
	}

	while (glfwGetWindowParam( GLFW_OPENED ))
	{
		ConstrainMouse();

		if (GameServer()->IsHalting())
		{
			DestroyGame();
			CreateGame(GAMETYPE_MENU);
		}

		float flTime = (float)glfwGetTime();
		if (GameServer())
		{
			if (GameServer()->IsLoading())
				CNetwork::Think();
			else if (GameServer()->IsClient() && !CNetwork::IsConnected())
			{
				DestroyGame();
				CreateGame(GAMETYPE_MENU);
			}
			else
			{
				GameServer()->Think(flTime);
				Render();
			}
		}
		else
			// Clear the buffer for the gui.
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		if (!GameServer() || GameServer()->IsLoading())
			RenderLoading();
		else
		{
			glgui::CRootPanel::Get()->Think(flTime);
			glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

			glfwSwapBuffers();
		}
	}
}

void CDigitanksWindow::ConstrainMouse()
{
#ifdef _WIN32
	if (IsFullscreen())
		return;

	HWND hWindow = FindWindow(NULL, L"Digitanks!");

	if (!hWindow)
		return;

	HWND hActiveWindow = GetActiveWindow();
	if (ShouldConstrainMouse() && hActiveWindow == hWindow && GameServer() && !GameServer()->IsLoading() && DigitanksGame()->GetGameType() != GAMETYPE_MENU && !GetMenu()->IsVisible())
	{
		RECT rc;
		GetClientRect(hWindow, &rc);

		// Convert the client area to screen coordinates.
		POINT pt = { rc.left, rc.top };
		POINT pt2 = { rc.right, rc.bottom };
		ClientToScreen(hWindow, &pt);
		ClientToScreen(hWindow, &pt2);
		SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);

		// Confine the cursor.
		ClipCursor(&rc);
	}
	else
		ClipCursor(NULL);
#endif
}

void CDigitanksWindow::Render()
{
	if (GameServer())
		GameServer()->Render();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	if (GameServer() && GameServer()->GetRenderer())
		GameServer()->GetRenderer()->SetSize(w, h);

	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (GameServer())
		Render();

	glgui::CRootPanel::Get()->Layout();
	glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	glfwSwapBuffers();
}

bool CDigitanksWindow::GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit, int iCollisionGroup)
{
	if (!DigitanksGame()->GetTerrain())
		return false;

	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	Vector vecWorld = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GameServer()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return GameServer()->GetGame()->TraceLine(vecCameraVector, vecCameraVector+vecRay*1000, vecPoint, pHit, iCollisionGroup);
}

void CDigitanksWindow::GameOver(bool bPlayerWon)
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	GetInstructor()->SetActive(false);
	m_pVictory->GameOver(bPlayerWon);
}

void CDigitanksWindow::CloseApplication()
{
	if (IsRegistered())
		exit(0);

	if (m_pPurchase->IsVisible())
		exit(0);

	m_pMenu->SetVisible(false);
	m_pMainMenu->SetVisible(false);
	m_pPurchase->ClosingApplication();

	SaveConfig();
}

void CDigitanksWindow::SaveConfig()
{
	c.add<float>("soundvolume", GetSoundVolume());
	c.add<float>("musicvolume", GetMusicVolume());
	c.add<bool>("windowed", !m_bCfgFullscreen);
	c.add<bool>("constrainmouse", m_bConstrainMouse);
	c.add<int>("width", m_iCfgWidth);
	c.add<int>("height", m_iCfgHeight);
	std::ofstream o;
	o.open(GetAppDataDirectory(L"Digitanks", L"options.cfg"), std::ios_base::out);
	o << c;
}

CInstructor* CDigitanksWindow::GetInstructor()
{
	if (!m_pInstructor)
		m_pInstructor = new CInstructor();

	return m_pInstructor;
}

bool CDigitanksWindow::HasCommandLineSwitch(const char* pszSwitch)
{
	for (size_t i = 0; i < m_apszCommandLine.size(); i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return true;
	}

	return false;
}

const char* CDigitanksWindow::GetCommandLineSwitchValue(const char* pszSwitch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_apszCommandLine.size()-1; i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return m_apszCommandLine[i+1];
	}

	return NULL;
}

void CDigitanksWindow::SetSoundVolume(float flSoundVolume)
{
	m_flSoundVolume = flSoundVolume;
	CSoundLibrary::SetSoundVolume(m_flSoundVolume);
}

void CDigitanksWindow::SetMusicVolume(float flMusicVolume)
{
	m_flMusicVolume = flMusicVolume;
	CSoundLibrary::SetMusicVolume(m_flMusicVolume);
}


#include "core/system.h"
#include "system/file.h"
#include "system/engine.h"
#include "render/texture.h"
#include "core/sound_mgr.h"
#include "util/drg_gc.h"
#include "util/stringn.h"
#include "util/settings.h"
#include "util/profile.h"
#include "file/file_manager.h"
#include "render/gui/gui.h"
#include "core/cmdarg.h"

//#include "script_pnet/ScriptInterface.h"

drgNativeAppInterfaceInit AppInterfaceNative::g_NativeAppInterfaceInit = NULL;
drgNativeAppInterfaceCleanUp AppInterfaceNative::g_NativeAppInterfaceCleanUp = NULL;
drgNativeAppInterfaceUpdate AppInterfaceNative::g_NativeAppInterfaceUpdate = NULL;

AppInterfaceNative::AppInterfaceNative(drgNativeAppInterfaceInit _init, drgNativeAppInterfaceCleanUp _cleanup, drgNativeAppInterfaceUpdate _update) {
	if (g_NativeAppInterfaceInit != NULL) {
		drgPrintError("Only One Native App!");
		assert(0);
	}
	g_NativeAppInterfaceInit = _init;
	g_NativeAppInterfaceCleanUp = _cleanup;
	g_NativeAppInterfaceUpdate = _update;
}



int drgSystem::m_LoadState = 0;

int drgSystem::Init()
{
	m_LoadState = 0;

	//drgXMLNode test_node = drgXMLUtil::ReadXMLFile("C:\\asset_drg\\projects\\ModelPreviewer\\Resources\\BA_Walk.wam");
	//test_node->WriteFile("C:\\asset_drg\\projects\\ModelPreviewer\\Resources\\BA_Walk_out.xml");

	//drgFile::CreateDataArray("C:\\asset_drg\\projects\\ACEed\\res_ACEed\\ui\\fonts\\consola_170.png", "C:\\asset_drg\\projects\\ACEed\\res_ACEed\\ui\\fonts\\consola_tga.cpp", "check_up_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\asset_drg\\projects\\ACEed\\res_ACEed\\ui\\fonts\\consola_17.fnt", "C:\\asset_drg\\projects\\ACEed\\res_ACEed\\ui\\fonts\\consola_fnt.cpp", "check_up_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\check_down.png", "C:\\Projects\\Bingo\\res_win\\icons\\check_down.cpp", "check_down_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\dir_up.png", "C:\\Projects\\Bingo\\res_win\\icons\\dir_up.cpp", "check_up_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\check_down.png", "C:\\Projects\\Bingo\\res_win\\icons\\check_down.cpp", "check_down_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\check.png", "C:\\Projects\\Bingo\\res_win\\icons\\check.cpp", "check_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\font1.png", "C:\\Projects\\Bingo\\res_win\\icons\\font1.cpp", "font_icon", drgFile::DRG_DATA_ARRAY_CPP);
	//drgFile::CreateDataArray("C:\\Projects\\Bingo\\res_win\\icons\\font2.png", "C:\\Projects\\Bingo\\res_win\\icons\\font2.cpp", "font_icon", drgFile::DRG_DATA_ARRAY_CPP);
//	drgFile::CreateDataArray("C:\\asset_drg\\script\\eso\\testbed.eso", "C:\\asset_drg\\engine\\f_testbed_eso.js", "testbed_eso", drgFile::DRG_DATA_ARRAY_EMSCRIPT);
	//drgFile::CreateDataArray("C:\\asset_drg\\script\\eso\\Vertigo.eso", "C:\\asset_drg\\engine\\f_vertigo_eso.js", "vertigo_eso", drgFile::DRG_DATA_ARRAY_EMSCRIPT);
	//drgFile::CreateDataArray("C:\\asset_drg\\script\\eso\\drglib.eso", "C:\\asset_drg\\engine\\f_drglib_eso.js", "drglib_eso", drgFile::DRG_DATA_ARRAY_EMSCRIPT);

	//unsigned int len = 0;
	//unsigned int count = 0;
	//char pathName[DRG_MAX_PATH];
	//drgString::Copy(pathName, drgFile::GetResourceFolder());
	//drgString::Concatenate( pathName, "hello.txt");
	//char* helloval = (char*)drgFile::MallocFile(pathName, &len);
	//drgPrintOut("FILE LEN: %u\r\n", len);
	//for(count=0; count<len; ++count)
	//	drgPrintOut("%c", helloval[count]);
	//drgPrintOut("\r\n");

	string8 settings_path = drgFile::GetResourceFolder();
	settings_path += "settings.ini";
	drgSettings::GetInstance(drgSettings::DRG_SETTINGS_ENGINE)->Init(settings_path.c_str());

	drgEngine::Init();
	DRG_PROFILE_INIT();
	drgFileManager::Init();
	drgSoundMgr::Init();
	AppInterface::Init();
	//drgNetManager::Init();
	DRG_PROFILE_DUMPDATA(true);


	//USE_SCRIPT_INTERFACE = false;
	//{
	//	drgXMLNode testnode = drgXMLUtil::ReadXMLFile("test.xml");
	//	drgTexture* tex = drgTexture::LoadFromFile("test.png", false);
	//}
	//g_MainBlocks.PrintInfo(true);
	//g_MainBlocks.SetAllocInfo(MEM_INFO_FLAG_USER_1);
	//g_MainBlocks.SetPrintInfo(MEM_INFO_FLAG_BLOCKS | MEM_INFO_FLAG_CHANGES | MEM_INFO_FLAG_USER_1);
	//{
	//	drgScene testscene;
	//	testscene.LoadSceneAsync("test.xml", NULL, NULL);
	//	while (testscene.AssetsLoadedPct() < 100)
	//	{
	//		drgThread::SleepCurrent(1);
	//		testscene.Update(0.1f);
	//	}
	//	//drgXMLNode m_SceneRootNode = drgXMLUtil::ReadXMLFile("test.xml");
	//	//drgXMLNode sceneDirectoryNode = m_SceneRootNode->GetChildNode("SCENE_FOLDER");
	//	//int numAssets = m_SceneRootNode.GetData()->GetNumChild("PROJECT_ASSETS");
	//}
	//drgEngine::ReleaseDelayedDeleteList();
	//g_MainBlocks.PrintInfo(true);

	const char* asset_server = drgCommandArg::GetArg("assetaddr");
	if (asset_server == NULL)
		asset_server = drgSettings::GetInstance(drgSettings::DRG_SETTINGS_ENGINE)->GetValueString("AssetAddr");
	if (asset_server != NULL)
		drgFileManager::SetAssetServer(asset_server);

	m_LoadState = 1;
	return 0;
}


int drgSystem::CleanUp()
{
	//drgNetManager::CleanUp();
	drgGarbageColector::GetDefault()->DeInit();
	drgSoundMgr::CleanUp();
	drgSettings::CleanUpAll();
	drgEngine::CleanUp();
	drgFileManager::CleanUp();
	return 0;
}

void drgSystem::Update()
{
	if (m_LoadState < 4)
	{  // still init state
		if (m_LoadState == 1)
		{ // checking network
			m_LoadState = 3;
			const char* asset_server = drgCommandArg::GetArg("assetaddr");
			if (asset_server == NULL)
				asset_server = drgSettings::GetInstance(drgSettings::DRG_SETTINGS_ENGINE)->GetValueString("AssetAddr");
			if (asset_server != NULL)
			{
				drgFileManager::SetAssetServer(asset_server);
				// int net_result = drgNetManager::CheckConnection(asset_server, 80);
				// if (net_result != 0)
					m_LoadState = 2;
			}
		}
		else if (m_LoadState == 2)
		{  // retry connect
			m_LoadState = 1;
		}
		else if (m_LoadState == 3)
		{ // load the script
			m_LoadState = 4;
			const char* script = drgCommandArg::GetArg("script");
			if (script == NULL)
				script = drgSettings::GetInstance(drgSettings::DRG_SETTINGS_ENGINE)->GetValueString("Script");
			if (script == NULL)
				script = "MOOSEed";
			AppInterface::Load(script);

			//g_MainBlocks.SetPrintInfo(MEM_INFO_FLAG_BLOCKS);
			//g_MainBlocks.PrintInfo(true, true, "C:\\ace\\runtime\\engine\\mem.csv");
			//g_MainBlocks.PrintInfo(false, false, NULL);

#if MEM_ENABLE_DEBUG
			g_MainBlocks.CheckInfo();
			g_MainBlocks.PrintInfo();
			g_MainBlocks.SetPrintInfo(MEM_INFO_FLAG_BLOCKS | MEM_INFO_FLAG_CHANGES | MEM_INFO_FLAG_CHECK_ONLY);
#endif
		}
		return;
	}

#if DRG_PROFILE
	static unsigned int countdown = 600;
	countdown--;
	if (countdown == 0)
	{
		DRG_PROFILE_DUMPDATA(true);
		countdown = 300;
	}
#endif

	DRG_PROFILE_FRAME();
	DRG_PROFILE_FUNK(drgSystem_Update, 0xFFFFFF00);

#if MEM_ENABLE_DEBUG
#if defined(_WINDOWS)
static unsigned int countdown = 1000;
countdown--;
if(countdown==0)
{
	// MEMORY BLOCK INFO PTR:0xf010020 SIZE:430080KB USED:295967KB FREE:134112KB BIGGEST:57080KB  COUNT:171879
	// MEMORY BLOCK INFO PTR:0xf010020 SIZE:430080KB USED:282867KB FREE:147212KB BIGGEST:93978KB  COUNT:64042     //xml fix
	// MEMORY BLOCK INFO PTR:0xf010020 SIZE:430080KB USED:231448KB FREE:198631KB BIGGEST:119120KB COUNT:57778     // command buffer destroy
	// MEMORY BLOCK INFO PTR:0xf010020 SIZE:430080KB USED:165703KB FREE:264376KB BIGGEST:159610KB COUNT:57661     // dynamic buffer
	// MEMORY BLOCK INFO PTR:0xf010020 SIZE:327680KB USED:161997KB FREE:165682KB BIGGEST:62438KB  COUNT:46378	

	//g_MainBlocks.PrintInfo(false);
	char filename[128];
	static int counting = 0;
	drgString::PrintFormated(filename, "C:\\ace\\mem_%i.csv", counting);
	g_MainBlocks.PrintInfo(true, true, filename);
	g_MainBlocks.PrintInfo(false);
	if (counting==0)
		g_MainBlocks.SetAllocInfo(MEM_INFO_FLAG_USER_1);
	else
		g_MainBlocks.SetAllocInfo(0);
	//g_MainBlocks.SetPrintInfo(MEM_INFO_FLAG_BLOCKS | MEM_INFO_FLAG_CHANGES | MEM_INFO_FLAG_USER_1);
	g_MainBlocks.SetPrintInfo(MEM_INFO_FLAG_BLOCKS);
	counting++;
	countdown = 800;
}
#endif
#endif

	drgVec3 panpos(0.0f, 0.0f, 0.0f);
	drgVec3 panvec(0.0f, 0.0f, 0.0f);

	//drgNetManager::Update();
	drgFileManager::Update();
	drgEngine::Update();
	double dtime = drgEngine::GetDeltaTime();
	drgSoundMgr::Update(dtime, panpos, panvec);
	//GUI_Dialog::UpdateDialogs();
	AppInterface::Update(dtime);
}

void drgSystem::Draw()
{
	drgEngine::Draw();
}





#include <assert.h>

#include <modelconverter/modelconverter.h>
#include "ui/modelwindow.h"
#include "crunch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>

typedef enum
{
	COMMAND_NONE = 0,
	COMMAND_AO,
} command_t;

class CPrintingWorkListener : public IWorkListener
{
public:
	virtual void BeginProgress() {};
	virtual void SetAction(wchar_t* pszAction, size_t iTotalProgress)
	{
		printf("\n");
		wprintf(pszAction);
		if (!iTotalProgress)
			printf("...");

		m_iTotalProgress = iTotalProgress;
		m_iProgress = 0;
	}

	virtual void WorkProgress(size_t iProgress, bool bForceDraw = false)
	{
		if (!m_iTotalProgress)
			return;

		size_t iLastProgress = m_iProgress;
		m_iProgress = iProgress;

		size_t iLastPercent = (iLastProgress * 10 / m_iTotalProgress);
		size_t iPercent = (iProgress * 10 / m_iTotalProgress);

		if (iPercent > iLastPercent)
		{
			printf("%d", iPercent);
			return;
		}

		iLastPercent = (iLastProgress * 40 / m_iTotalProgress);
		iPercent = (iProgress * 40 / m_iTotalProgress);

		if (iPercent > iLastPercent)
			printf(".");
	}

	virtual void EndProgress()
	{
		printf("\n");
	}

public:
	size_t					m_iProgress;
	size_t					m_iTotalProgress;
};

int main(int argc, char** argv)
{
	std::wstring sFile;
	command_t eCommand = COMMAND_NONE;
	aomethod_t eMethod = AOMETHOD_SHADOWMAP;
	size_t iSize = 1024;
	size_t iBleed = 1;
	size_t iLights = 3000;
	size_t iSamples = 20;
	float flRayFalloff = 1.0f;
	bool bRandomize = false;
	bool bCrease = false;
	bool bGroundOcclusion = false;
	std::wstring sOutput;

	if (argc >= 2)
	{
		for (int i = 1; i < argc; i++)
		{
			wchar_t szToken[1024];
			mbstowcs(szToken, argv[i], strlen(argv[i])+1);

			if (szToken[0] == L'-')
			{
				// It's an argument
				if (wcscmp(szToken, L"--command") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					if (wcscmp(szToken, L"ao") == 0)
						eCommand = COMMAND_AO;
				}
				else if (wcscmp(szToken, L"--method") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					if (wcscmp(szToken, L"shadowmap") == 0)
						eMethod = AOMETHOD_SHADOWMAP;
					else if (wcscmp(szToken, L"raytrace") == 0)
						eMethod = AOMETHOD_RAYTRACE;
					else if (wcscmp(szToken, L"tridistance") == 0)
						eMethod = AOMETHOD_TRIDISTANCE;
					else if (wcscmp(szToken, L"color") == 0)
						eMethod = AOMETHOD_RENDER;
					else
						printf("ERROR: Unrecognized method.\n");
				}
				else if (wcscmp(szToken, L"--size") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					iSize = _wtoi(szToken);
					if (iSize < 64)
						iSize = 64;
					else if (iSize > 2048)
						iSize = 2048;
				}
				else if (wcscmp(szToken, L"--bleed") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					iBleed = _wtoi(szToken);
					if (iBleed < 0)
						iBleed = 0;
					else if (iBleed > 10)
						iBleed = 10;
				}
				else if (wcscmp(szToken, L"--lights") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					iLights = _wtoi(szToken);
					if (iLights < 500)
						iLights = 500;
					else if (iSize > 3000)
						iLights = 3000;
				}
				else if (wcscmp(szToken, L"--samples") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					iSamples = _wtoi(szToken);
					if (iSamples < 5)
						iSamples = 5;
					else if (iSamples > 25)
						iSamples = 25;
				}
				else if (wcscmp(szToken, L"--falloff") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					if (wcscmp(szToken, L"none") == 0)
						flRayFalloff = -1.0f;
					else
					{
						flRayFalloff = (float)_wtof(szToken);
						if (flRayFalloff < 0.0001f)
							flRayFalloff = 0.0001f;
					}
				}
				else if (wcscmp(szToken, L"--randomize") == 0)
				{
					bRandomize = true;
				}
				else if (wcscmp(szToken, L"--crease") == 0)
				{
					bCrease = true;
				}
				else if (wcscmp(szToken, L"--groundocclusion") == 0)
				{
					bGroundOcclusion = true;
				}
				else if (wcscmp(szToken, L"--output") == 0)
				{
					i++;
					mbstowcs(szToken, argv[i], strlen(argv[i])+1);
					sOutput = std::wstring(szToken);
				}
			}
			else
			{
				// It's a file
				sFile = std::wstring(szToken);
			}
		}

		switch (eCommand)
		{
		case COMMAND_AO:
		{
			wprintf(L"Generating ambient occlusion map for %s\n", sFile.c_str());
			switch (eMethod)
			{
			case AOMETHOD_RENDER:
				printf("Method: Color AO\n");
				break;

			case AOMETHOD_TRIDISTANCE:
				printf("Method: Triangle distance\n");
				break;

			case AOMETHOD_RAYTRACE:
				printf("Method: Raytrace\n");
				break;

			case AOMETHOD_SHADOWMAP:
				printf("Method: Shadow mapping\n");
				break;
			}
			printf("Size: %dx%d\n", iSize, iSize);
			printf("Bleed: %d\n", iBleed);
			if (eMethod == AOMETHOD_SHADOWMAP)
				printf("Lights: %d\n", iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				printf("Samples: %d\n", iSamples);
			wprintf(L"Output file: %s\n", sOutput.c_str());

			CConversionScene s;
			CModelConverter c(&s);

			CPrintingWorkListener l;

			c.SetWorkListener(&l);

			if (!c.ReadModel(sFile.c_str()))
			{
				printf("Unsupported model format.\n");
				return 1;
			}

			if (eMethod == AOMETHOD_SHADOWMAP || eMethod == AOMETHOD_RENDER)
			{
				glutInit(&argc, argv);
				glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);

				// The easy way to get a "windowless" context.
				glutCreateWindow("SMAK a Batch");
				glutHideWindow();

				glewInit();
			}

			ilInit();

			// If this is the color AO method, we need to load the textures.
			std::vector<CMaterial> aMaterials;
			CModelWindow::LoadSMAKTexture();
			if (eMethod == AOMETHOD_RENDER)
			{
				for (size_t i = 0; i < s.GetNumMaterials(); i++)
				{
					CConversionMaterial* pMaterial = s.GetMaterial(i);

					aMaterials.push_back(CMaterial(0));

					assert(aMaterials.size()-1 == i);

					size_t iTexture = CModelWindow::LoadTextureIntoGL(pMaterial->GetDiffuseTexture());

					if (iTexture)
						aMaterials[i].m_iBase = iTexture;
				}

				if (!aMaterials.size())
				{
					aMaterials.push_back(CMaterial(0));
				}
			}

			CAOGenerator ao(&s, &aMaterials);

			ao.SetMethod(eMethod);
			ao.SetSize(iSize, iSize);
			ao.SetBleed(iBleed);
			ao.SetUseTexture(true);
			if (eMethod == AOMETHOD_SHADOWMAP)
				ao.SetSamples(iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				ao.SetSamples(iSamples);
			ao.SetRandomize(bRandomize);
			ao.SetCreaseEdges(bCrease);
			ao.SetGroundOcclusion(bGroundOcclusion);

			ao.SetWorkListener(&l);

			ao.Generate();

			printf("Saving results...\n");
			if (wcslen(sOutput.c_str()))
				ao.SaveToFile(sOutput.c_str());
			else
				ao.SaveToFile(L"ao-output.png");

			printf("Done.\n");
			return 0;
		}
		}
	}

	CModelWindow oWindow;

	if (sFile.length())
		oWindow.ReadFile(sFile.c_str());

	oWindow.Run();
}

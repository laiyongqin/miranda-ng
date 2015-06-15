#include "stdafx.h"

CLuaOptions::CLuaOptions(int idDialog)
	: CDlgBase(g_hInstance, idDialog), m_scripts(this, IDC_SCRIPTS)
{
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, type, iValue);
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, szValue);
}

void CLuaOptions::LoadScripts(const TCHAR *scriptDir)
{
	TCHAR searchMask[MAX_PATH];
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));

	TCHAR fullPath[MAX_PATH], path[MAX_PATH];

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				mir_sntprintf(fullPath, _T("%s\\%s"), scriptDir, fd.cFileName);
				PathToRelativeT(fullPath, path);

				m_scripts.AddItem(fd.cFileName, -1, NULL, 0);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CLuaOptions::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scripts.EnableGroupView(TRUE);
	m_scripts.AddGroup(0, TranslateT("Common scripts"));
	m_scripts.AddGroup(1, TranslateT("Custom scripts"));

	m_scripts.AddColumn(0, _T("Script"), 300);

	WIN32_FIND_DATA fd;
	HANDLE hFind = NULL;
	TCHAR scriptDir[MAX_PATH];
	TCHAR searchMask[MAX_PATH];

	FoldersGetCustomPathT(g_hCommonFolderPath, scriptDir, SIZEOF(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));
	hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				m_scripts.AddItem(fd.cFileName, -1, NULL, 0);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	FoldersGetCustomPathT(g_hCustomFolderPath, scriptDir, SIZEOF(scriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));
	hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				m_scripts.AddItem(fd.cFileName, -1, NULL, 1);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}
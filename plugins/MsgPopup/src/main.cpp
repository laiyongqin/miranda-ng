/*

MessagePopup - replacer of MessageBox'es

Copyright 2004 Denis Stanishevskiy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

CMPlugin g_plugin;

MSGBOXOPTIONS optionsDefault =
{
	{0, 0xFFFFFF, 0, 0},
	{0xBFFFFF, 0x0000FF, 0xB0B0FF, 0xECFF93},
	{-1,-1,-1,-1},
	TRUE
};
MSGBOXOPTIONS options;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {CF25D645-4DAB-4B0A-B9F1-DE1E86231F9B}
	{0xcf25d645, 0x4dab, 0x4b0a, {0xb9, 0xf1, 0xde, 0x1e, 0x86, 0x23, 0x1f, 0x9b}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

typedef int (WINAPI *MSGBOXPROC)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

MSGBOXPROC prevMessageBox;

#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516

void popupMessage(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	POPUPDATAT ppd = { 0 };
	int iIcon;
	int indx;

	switch (uType & 0xF0) {
	case MB_ICONHAND:
		indx = 1;
		iIcon = OIC_HAND;
		break;
	case MB_ICONEXCLAMATION:
		indx = 2;
		iIcon = OIC_BANG;
		break;
	case MB_ICONQUESTION:
		indx = 3;
		iIcon = OIC_QUES;
		break;
	default:
		indx = 0;
		iIcon = OIC_NOTE;
		break;

	}
	ppd.colorBack = options.BG[indx];
	ppd.colorText = options.FG[indx];
	ppd.iSeconds = options.Timeout[indx];

	ppd.lchIcon = (HICON)LoadImage(nullptr, MAKEINTRESOURCE(iIcon), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_SHARED);
	mir_wstrcpy(ppd.lptzContactName, lpCaption);
	mir_wstrcpy(ppd.lptzText, lpText);
	PUAddPopupT(&ppd);
	if (options.Sound)
		MessageBeep(uType);
}

int WINAPI newMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	if (CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == CALLSERVICE_NOTFOUND || (uType & 0x0F))
		return prevMessageBox(hWnd, lpText, lpCaption, uType);

	popupMessage(lpText, lpCaption, uType);
	return IDOK;
}

BOOL g_HookError = FALSE;
BOOL g_HookError2 = FALSE;
int  g_mod = 0;

void HookOnImport(HMODULE hModule, char *lpszImpModName, PVOID lpOrigFunc, PVOID lpNewFunc)
{
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)
		ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
	if (pImportDesc == nullptr)
		return;

	for (; pImportDesc->Name; pImportDesc++) {
		char *pszModName = (char *)((PBYTE)hModule + pImportDesc->Name);
		if (mir_strcmpi(lpszImpModName, pszModName) != 0)
			continue;

		PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->FirstThunk);
		for (; pThunk->u1.Function; pThunk++) {
			PVOID* ppfn = (PVOID*)&pThunk->u1.Function;
			if (*ppfn != lpOrigFunc)
				continue;

			DWORD oldProtect;

			g_mod++;

			if (!VirtualProtect((LPVOID)ppfn, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
				if (!g_HookError) {
					wchar_t buf[200];

					g_HookError = TRUE;
					mir_snwprintf(buf, TranslateT("VirtualProtect failed. Code %d\nTry to call the author"), GetLastError());
					prevMessageBox(nullptr, buf, TranslateT("Error"), MB_OK);
				}
			}
			*(PVOID*)ppfn = lpNewFunc;
			if (*(PVOID*)ppfn != lpNewFunc) {
				if (!g_HookError2) {
					g_HookError2 = TRUE;
					prevMessageBox(nullptr, TranslateT("Hmm. Something goes wrong. I can't write into the memory.\nAnd as you can see, there are no any exception raised...\nTry to call the author"), TranslateT("Error"), MB_OK);
				}
			}
		}
	}
}

void HookAPI()
{
	PVOID lpMessageBox = (PVOID)GetProcAddress(GetModuleHandle(L"USER32.DLL"), "MessageBoxW");
	PVOID lpPopupMsgBox = (PVOID)newMessageBox;

	prevMessageBox = (MSGBOXPROC)lpMessageBox;

	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (hModuleSnap == INVALID_HANDLE_VALUE)
		return;

	MODULEENTRY32 me32 = { 0 };
	me32.dwSize = sizeof(MODULEENTRY32);

	BOOL bFound = FALSE;
	if (Module32First(hModuleSnap, &me32)) {
		do {
			HookOnImport(me32.hModule, "USER32.DLL", lpMessageBox, lpPopupMsgBox);
		}
			while (!bFound && Module32Next(hModuleSnap, &me32));
	}
	CloseHandle(hModuleSnap);
}

int HookedInit(WPARAM, LPARAM)
{
	HookAPI();
	return 0;
}

int HookedOptions(WPARAM wParam, LPARAM)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = {};
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
		odp.szTitle.w = LPGENW("MessagePopup");
		odp.szGroup.w = LPGENW("Popups");
		odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
		odp.pfnDlgProc = OptionsDlgProc;
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}

void LoadConfig()
{
	for (int indx = 0; indx < 4; indx++) {
		char szNameFG[4], szNameBG[4], szNameTO[4];
		mir_snprintf(szNameFG, "FG%d", indx);
		mir_snprintf(szNameBG, "BG%d", indx);
		mir_snprintf(szNameTO, "TO%d", indx);
		options.FG[indx] = g_plugin.getDword(szNameFG, optionsDefault.FG[indx]);
		options.BG[indx] = g_plugin.getDword(szNameBG, optionsDefault.BG[indx]);
		options.Timeout[indx] = g_plugin.getDword(szNameTO, (DWORD)optionsDefault.Timeout[indx]);
	}

	options.Sound = g_plugin.getByte("Sound", (DWORD)optionsDefault.Sound);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
	HookEvent(ME_OPT_INITIALISE, HookedOptions);

	LoadConfig();
	return 0;
}

/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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
#include "services.h"

int InitServices()
{
	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowContactHistoryService);
	return 0;
}

INT_PTR ShowContactHistoryService(WPARAM wParam, LPARAM)
{
	Log("%s", "Entering function " __FUNCTION__);
	HWND historyDlg;
	HWND parent = nullptr;
	historyDlg = WindowList_Find(hOpenWindowsList, (MCONTACT)wParam);
	if (historyDlg == nullptr){
		int count = g_plugin.getDword("EventsToLoad", 0);
		int loadInBackground = g_plugin.getByte("UseWorkerThread", 0);
		HistoryWindowData *data;
		data = (HistoryWindowData *)malloc(sizeof(HistoryWindowData));
		data->contact = (MCONTACT)wParam;
		data->hIEView = nullptr;
		data->itemsPerPage = count;
		data->index = 0;
		data->count = 0;
		data->loadMethod = (loadInBackground) ? LOAD_IN_BACKGROUND : 0;
		historyDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HISTORY), parent, HistoryDlgProc);
		SetWindowLongPtr(historyDlg, DWLP_USER, (LONG_PTR)data);

		WindowList_Add(hOpenWindowsList, historyDlg, (MCONTACT)wParam);
	}
	else
		SetForegroundWindow(historyDlg);
	ShowWindow(historyDlg, SW_SHOWNORMAL);
	return 0;
}

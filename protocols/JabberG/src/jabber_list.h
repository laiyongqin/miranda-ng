/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (�) 2012-17 Miranda NG project

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

#ifndef _JABBER_LIST_H_
#define _JABBER_LIST_H_

#include "jabber_caps.h"

#define LISTFOREACH(var__, obj__, list__)	\
	for (int var__ = 0; (var__ = obj__->ListFindNext(list__, var__)) >= 0; ++var__)
#define LISTFOREACH_NODEF(var__, obj__, list__)	\
	for (var__ = 0; (var__ = obj__->ListFindNext(list__, var__)) >= 0; ++var__)

enum JABBER_LIST
{
	LIST_ROSTER,        // Roster list
	LIST_CHATROOM,      // Groupchat room currently joined
	LIST_ROOM,          // Groupchat room list to show on the Jabber groupchat dialog
	LIST_FILE,          // Current file transfer session
	LIST_BYTE,          // Bytestream sending connection
	LIST_FTRECV,
	LIST_BOOKMARK,
	LIST_VCARD_TEMP,
	LIST_FTIQID
};

enum JABBER_SUBSCRIPTION
{
	SUB_NONE,
	SUB_TO,
	SUB_FROM,
	SUB_BOTH
};

enum JABBER_GC_AFFILIATION
{
	AFFILIATION_NONE,
	AFFILIATION_OUTCAST,
	AFFILIATION_MEMBER,
	AFFILIATION_ADMIN,
	AFFILIATION_OWNER
};

enum JABBER_GC_ROLE
{
	ROLE_NONE,
	ROLE_VISITOR,
	ROLE_PARTICIPANT,
	ROLE_MODERATOR
};

enum JABBER_RESOURCE_MODE // initial default to RSMODE_LASTSEEN
{
	RSMODE_SERVER,		// always let server decide (always send correspondence without resouce name)
	RSMODE_LASTSEEN,	// use the last seen resource (or let server decide if haven't seen anything yet)
	RSMODE_MANUAL		// specify resource manually (see the defaultResource field - must not be NULL)
};

class JABBER_RESOURCE_STATUS : public MZeroedObject
{
	LONG m_refCount;

public:
	JABBER_RESOURCE_STATUS();
	~JABBER_RESOURCE_STATUS();

	void AddRef();
	void Release();

	int    m_iStatus;
	ptrW   m_tszResourceName;
	ptrW   m_tszStatusMessage;
	int    m_iPriority; // resource priority, -128..+127
	time_t m_dwIdleStartTime;// XEP-0012 support

	// groupchat support
	JABBER_GC_AFFILIATION m_affiliation;
	JABBER_GC_ROLE m_role;
	ptrW  m_tszNick;
	ptrW  m_tszRealJid; // real jid for jabber conferences

	// XEP-0115 support
	ptrW  m_tszCapsNode;
	ptrW  m_tszCapsVer;
	ptrW  m_tszCapsExt;
	DWORD m_dwVersionRequestTime, m_dwDiscoInfoRequestTime;

	JabberCapsBits m_jcbCachedCaps;
	JabberCapsBits m_jcbManualDiscoveredCaps;

	// XEP-232 support
	ptrW  m_tszOs, m_tszOsVersion;
	ptrW  m_tszSoftware, m_tszSoftwareVersion, m_tszXMirandaCoreVersion;

	// XEP-0085 gone event support
	BOOL m_bMessageSessionActive;
};

class pResourceStatus
{
	JABBER_RESOURCE_STATUS *m_pStatus;

public:
	__forceinline pResourceStatus(JABBER_RESOURCE_STATUS *pStatus) :
		m_pStatus(pStatus)
	{	pStatus->AddRef();
	}

	__forceinline pResourceStatus(const pResourceStatus &r)
	{	m_pStatus = r.m_pStatus;
		m_pStatus->AddRef();
	}

	__forceinline ~pResourceStatus()
	{	m_pStatus->Release();
	}

	__forceinline operator JABBER_RESOURCE_STATUS*() const { return m_pStatus; }
	__forceinline JABBER_RESOURCE_STATUS* operator->() const { return m_pStatus; }

	__forceinline void operator=(const pResourceStatus &r) {
		m_pStatus->Release();
		m_pStatus = r.m_pStatus;
		m_pStatus->AddRef();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct JABBER_LIST_ITEM : public MZeroedObject
{
	JABBER_LIST_ITEM();
	~JABBER_LIST_ITEM();

	JABBER_LIST list;
	wchar_t* jid;
	MCONTACT hContact;

	// LIST_ROSTER
	// jid = jid of the contact
	wchar_t* nick;

	pResourceStatus findResource(const wchar_t *resourceName) const;
	pResourceStatus getBestResource() const;
	JABBER_RESOURCE_MODE resourceMode;
	LIST<JABBER_RESOURCE_STATUS> arResources; // array of resources
	JABBER_RESOURCE_STATUS
		*m_pLastSeenResource, // resource which was last seen active
		*m_pManualResource,   // manually set resource
		*m_pItemResource,     // resource for jids without /resource node
		*getTemp();           // allocates m_pItemResource if needed

	JABBER_SUBSCRIPTION subscription;
	wchar_t* group;
	wchar_t* photoFileName;
	wchar_t* messageEventIdStr;

	// LIST_AGENT
	// jid = jid of the agent
	wchar_t* name;
	wchar_t* service;

	// LIST_ROOM
	// jid = room JID
	wchar_t* type;	// room type

	// LIST_CHATROOM
	// jid = room JID
	BOOL bChatActive;
	HWND hwndGcListBan;
	HWND hwndGcListAdmin;
	HWND hwndGcListOwner;
	int  iChatState;

	// LIST_FILE
	// jid = string representation of port number
	filetransfer *ft;
	WORD port;

	// LIST_BYTE
	// jid = string representation of port number
	JABBER_BYTE_TRANSFER *jbt;

	JABBER_IBB_TRANSFER *jibb;

	// LIST_FTRECV
	// jid = string representation of stream id (sid)
	// ft = file transfer data

	//LIST_BOOKMARK
	// jid = room JID
	wchar_t* password;	// password for room
	bool bAutoJoin;

	bool bUseResource;
	bool bHistoryRead;
	bool bRealContact;
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct JABBER_HTTP_AVATARS
{
	char * Url;
	MCONTACT hContact;

	JABBER_HTTP_AVATARS(const wchar_t *tUrl, MCONTACT thContact)
		: Url(mir_u2a(tUrl)), hContact(thContact) {}

	~JABBER_HTTP_AVATARS() { mir_free(Url); }

	static int compare(const JABBER_HTTP_AVATARS *p1, const JABBER_HTTP_AVATARS *p2)
	{ return mir_strcmp(p1->Url, p2->Url); }
};

#endif

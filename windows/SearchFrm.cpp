/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"

#include "Resource.h"
#include "SearchFrm.h"
#include "MainFrm.h"
#include "BarShader.h"
#include "CustomDrawHelpers.h"

#include "../client/QueueManager.h"
#include "../client/SearchQueue.h"
#include "../client/ClientManager.h"
#include "../client/ShareManager.h"
#include "../client/DownloadManager.h"
#include "../client/PortTest.h"

std::list<wstring> SearchFrame::g_lastSearches;
HIconWrapper SearchFrame::g_purge_icon(IDR_PURGE);
HIconWrapper SearchFrame::g_pause_icon(IDR_PAUSE);
HIconWrapper SearchFrame::g_search_icon(IDR_SEARCH);
HIconWrapper SearchFrame::g_UDPOkIcon(IDR_ICON_SUCCESS_ICON);
HIconWrapper SearchFrame::g_UDPFailIcon(IDR_ICON_FAIL_ICON);
HIconWrapper SearchFrame::g_UDPWaitIcon(IDR_ICON_WARN_ICON);

extern bool g_DisableTestPort;

int SearchFrame::columnIndexes[] =
{
	COLUMN_FILENAME,
	COLUMN_LOCAL_PATH,
	COLUMN_HITS,
	COLUMN_NICK,
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
	COLUMN_ANTIVIRUS,
#endif
	COLUMN_P2P_GUARD,
	COLUMN_TYPE,
	COLUMN_SIZE,
	COLUMN_PATH,
	COLUMN_FLY_SERVER_RATING,
	COLUMN_BITRATE,
	COLUMN_MEDIA_XY,
	COLUMN_MEDIA_VIDEO,
	COLUMN_MEDIA_AUDIO,
	COLUMN_DURATION,
	COLUMN_SLOTS,
	COLUMN_HUB,
	COLUMN_EXACT_SIZE,
	COLUMN_LOCATION,
	COLUMN_IP,
#ifdef FLYLINKDC_USE_DNS
	COLUMN_DNS, // !SMT!-IP
#endif
	COLUMN_TTH,
	COLUMN_TORRENT_COMMENT,
	COLUMN_TORRENT_DATE,
	COLUMN_TORRENT_URL,
	COLUMN_TORRENT_TRACKER,
	COLUMN_TORRENT_PAGE
};

int SearchFrame::columnSizes[] =
{
	210,
//70,
	80, 100, 50,
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
	50,
#endif
	80, 100, 40,
// COLUMN_FLY_SERVER_RATING
	50,
	50, 100, 100, 100,
// COLUMN_DURATION
	30,
	150, 80, 80,
	100,
	100,
#ifdef FLYLINKDC_USE_DNS
	100,
#endif
	150,
	40,
	30,
	40,
	100,
	40,
	20
}; // !SMT!-IP

static ResourceManager::Strings columnNames[] = {ResourceManager::FILE,
                                                 ResourceManager::LOCAL_PATH,
                                                 ResourceManager::HIT_COUNT,
                                                 ResourceManager::USER,
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
                                                 ResourceManager::ANTIVIRUS,
#endif
                                                 ResourceManager::TYPE,
                                                 ResourceManager::SIZE,
                                                 ResourceManager::PATH,
                                                 ResourceManager::FLY_SERVER_RATING, // COLUMN_FLY_SERVER_RATING
                                                 ResourceManager::BITRATE,
                                                 ResourceManager::MEDIA_X_Y,
                                                 ResourceManager::MEDIA_VIDEO,
                                                 ResourceManager::MEDIA_AUDIO,
                                                 ResourceManager::DURATION,
                                                 ResourceManager::SLOTS,
//[-]PPA    , ResourceManager::CONNECTION,
                                                 ResourceManager::HUB,
                                                 ResourceManager::EXACT_SIZE,
//[-]PPA        ResourceManager::AVERAGE_UPLOAD,
                                                 ResourceManager::LOCATION_BARE,
                                                 ResourceManager::IP,
#ifdef FLYLINKDC_USE_DNS
                                                 ResourceManager::DNS_BARE, // !SMT!-IP
#endif
                                                 ResourceManager::TTH_ROOT_OR_TORRENT_MAGNET,
                                                 ResourceManager::P2P_GUARD,       // COLUMN_P2P_GUARD
                                                 ResourceManager::TORRENT_COMMENT,
                                                 ResourceManager::TORRENT_DATE,
                                                 ResourceManager::TORRENT_URL,
                                                 ResourceManager::TORRENT_TRACKER,
                                                 ResourceManager::TORRENT_PAGE
                                                };

SearchFrame::FrameMap SearchFrame::g_search_frames;

SearchFrame::SearchFrame() :
	CFlyTimerAdapter(m_hWnd),
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
	CFlyServerAdapter(7000),
#endif
	searchBoxContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	searchContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP),
	sizeContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP),
	modeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	sizeModeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	fileTypeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//m_treeContainer(WC_TREEVIEW, this, SEARH_TREE_MESSAGE_MAP),
	
	showUIContainer(WC_COMBOBOX, this, SHOWUI_MESSAGE_MAP),
	//slotsContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//collapsedContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
	//storeIPContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	m_storeIP(false),
#endif
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
	//m_FlyServerContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//m_FlyServerGradientContainer(WC_STATIC, this, SEARCH_MESSAGE_MAP),
#endif
	//storeSettingsContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//purgeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//doSearchContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	//doSearchPassiveContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
	resultsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
	hubsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
	ctrlFilterContainer(WC_EDIT, this, SEARCH_FILTER_MESSAGE_MAP),
	ctrlFilterSelContainer(WC_COMBOBOX, this, SEARCH_FILTER_MESSAGE_MAP),
	m_initialSize(0), m_initialMode(Search::SIZE_ATLEAST), m_initialType(FILE_TYPE_ANY),
	m_showUI(true), onlyFree(false), m_isHash(false), droppedResults(0), resultsCount(0),
	m_expandSR(false),
	m_storeSettings(false), m_isExactSize(false), m_exactSize2(0), /*searches(0),*/
	m_lastFindTTH(false),
	m_running(false),
	m_searchEndTime(0),
	m_searchStartTime(0),
	m_waitingResults(false),
	needUpdateResultCount(false),
	m_is_expand_tree(false),
	m_is_expand_sub_tree(false),
	m_Theme(nullptr),
	m_need_resort(false),
	m_is_use_tree(true),
	m_is_before_search(false),
	m_RootTreeItem(nullptr),
	m_CurrentTreeItem(nullptr),
	m_OldTreeItem(nullptr),
	m_is_disable_torrent_RSS(false),
	portStatus(PortTest::STATE_UNKNOWN)
{
}

SearchFrame::~SearchFrame()
{
	//dcassert(m_search_info_leak_detect.empty());
	dcassert(m_si_set.empty());
	dcassert(closed);
	images.Destroy();
	m_searchTypesImageList.Destroy();
// ���� �� ���� OperaColors::ClearCache();
}

inline static bool isTTHChar(const TCHAR c)
{
	return (c >= _T('2') && c <= _T('7')) || (c >= _T('A') && c <= _T('Z'));
}

static bool isTTH(const tstring& tth)
{
	if (tth.size() != 39)
		return false;
	for (size_t i = 0; i < 39; i++)
		if (!isTTHChar(tth[i]))
			return false;
	return true;
}

void SearchFrame::openWindow(const tstring& str /* = Util::emptyString */, LONGLONG size /* = 0 */, Search::SizeModes mode /* = Search::SIZE_ATLEAST */, int type /* = FILE_TYPE_ANY */)
{
	SearchFrame* pChild = new SearchFrame();
	pChild->m_is_disable_torrent_RSS = !str.empty();
	pChild->setInitial(str, size, mode, type);
	pChild->CreateEx(WinUtil::g_mdiClient);
	g_search_frames.insert(FramePair(pChild->m_hWnd, pChild));
}

void SearchFrame::closeAll()
{
	dcdrun(const auto l_size_g_frames = g_search_frames.size());
	for (auto i = g_search_frames.cbegin(); i != g_search_frames.cend(); ++i)
	{
		::PostMessage(i->first, WM_CLOSE, 0, 0);
	}
	dcassert(l_size_g_frames == g_search_frames.size());
}

LRESULT SearchFrame::onFiletypeChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BOOLSETTING(SAVE_SEARCH_SETTINGS))
	{
		SET_SETTING(SAVED_SEARCH_TYPE, ctrlFiletype.GetCurSel());
		SET_SETTING(SAVED_SEARCH_SIZEMODE, ctrlSizeMode.GetCurSel());
		SET_SETTING(SAVED_SEARCH_MODE, ctrlMode.GetCurSel());
		tstring st;
		WinUtil::getWindowText(ctrlSize, st);
		SET_SETTING(SAVED_SEARCH_SIZE, Text::fromT(st));
	}
	onSizeMode();
	return 0;
}

void SearchFrame::onSizeMode()
{
	const BOOL l_is_normal = ctrlMode.GetCurSel() != 0;
	::EnableWindow(GetDlgItem(IDC_SEARCH_SIZE), l_is_normal);
	::EnableWindow(GetDlgItem(IDC_SEARCH_SIZEMODE), l_is_normal);
}

LRESULT SearchFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
#if 0 // ???
	CFlyServerConfig::loadTorrentSearchEngine();
#endif
#ifdef FLYLINKDC_USE_TORRENT_PANEL
	{
		CRect rcVert;
		GetClientRect(&rcVert);
		
		// create the vertical splitter
		m_vSplit.Create(m_hWnd, rcVert, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		
		// set the vertical splitter parametersf
		m_vSplit.m_cxyMin = 222; // minimum size
		m_vSplit.SetSplitterPos(222); // from left
		//m_vSplit.m_bFullDrag = false;  // ghost bar enabled
		
		CRect rcHorz;
		GetClientRect(&rcHorz);
		
		// create the horizontal splitter. Note that vSplit is parent of hzSplit
		m_hzSplit.Create(m_vSplit, rcHorz, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		
		// set the horizontal splitter parameters
		m_hzSplit.m_cxyMin = 35; // minimum size
		m_hzSplit.SetSplitterPos(100); // from top
		m_hzSplit.m_bFullDrag = false; // ghost bar enabled
		
		// add the horizontal splitter to right pane of vertical splitter
		m_vSplit.SetSplitterPane(1, m_hzSplit);
		m_lPane.Create(m_vSplit.m_hWnd);
		// add container to left pane (0) of vertical splitter
		m_vSplit.SetSplitterPane(0, m_lPane);
		
		// set the left pane title
		//m_lPane.SetTitle(_T("Left Pane"));
		//m_lPane.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);
		
		// create the top container.  Note use of hzSplit as parent
		m_tPane.Create(m_hzSplit.m_hWnd);
		
		// add container to top pane (0) of horizontal splitter
		m_hzSplit.SetSplitterPane(0, m_tPane);
		
		// set the top pane title
		m_tPane.SetTitle(_T("Top Pane -- no Close button"));
		
		// remove the close button from the top container
		m_tPane.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);
		
	}
	HWND l_lHwnd = m_lPane;
	HWND l_SearchHwnd = m_tPane;
#else
	HWND l_lHwnd = m_hWnd;
#endif
	m_tooltip.Create(m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP /*| TTS_BALLOON*/, WS_EX_TOPMOST);
	m_tooltip.SetDelayTime(TTDT_AUTOPOP, 15000);
	dcassert(m_tooltip.IsWindow());
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlSearchBox.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                     WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, 0);
	CFlylinkDBManager::getInstance()->load_registry(g_lastSearches, e_SearchHistory); //[+]PPA
	init_last_search_box();
	searchBoxContainer.SubclassWindow(ctrlSearchBox.m_hWnd);
	ctrlSearchBox.SetExtendedUI();
	
	ctrlMode.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE, IDC_SEARCH_MODE);
	modeContainer.SubclassWindow(ctrlMode.m_hWnd);
	
	ctrlSize.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                ES_AUTOHSCROLL | ES_NUMBER, WS_EX_CLIENTEDGE, IDC_SEARCH_SIZE);
	sizeContainer.SubclassWindow(ctrlSize.m_hWnd);
	
	ctrlSizeMode.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                    WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE, IDC_SEARCH_SIZEMODE);
	sizeModeContainer.SubclassWindow(ctrlSizeMode.m_hWnd);
	
	ctrlFiletype.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                    WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED, WS_EX_CLIENTEDGE, IDC_FILETYPES);
	                    
	ResourceLoader::LoadImageList(IDR_SEARCH_TYPES, m_searchTypesImageList, 16, 16);
	fileTypeContainer.SubclassWindow(ctrlFiletype.m_hWnd);
	
	const bool useSystemIcon = BOOLSETTING(USE_SYSTEM_ICONS);
	
	if (useSystemIcon)
	{
		ctrlResults.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		                   WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS // | LVS_EX_INFOTIP
		                   , WS_EX_CLIENTEDGE, IDC_RESULTS);
	}
	else
	{
		ctrlResults.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		                   WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS // | LVS_EX_INFOTIP
		                   , WS_EX_CLIENTEDGE, IDC_RESULTS);
	}
	ctrlResults.m_is_managed = true;
	// m_tPane.SetClient(ctrlResults);
	
#ifdef FLYLINKDC_USE_TREE_SEARCH
	m_ctrlSearchFilterTree.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_TRANSFER_TREE);
	m_ctrlSearchFilterTree.SetBkColor(Colors::g_bgColor);
	m_ctrlSearchFilterTree.SetTextColor(Colors::g_textColor);
	WinUtil::SetWindowThemeExplorer(m_ctrlSearchFilterTree.m_hWnd);
	m_ctrlSearchFilterTree.SetImageList(m_searchTypesImageList, TVSIL_NORMAL);
	
	//m_treeContainer.SubclassWindow(m_ctrlSearchFilterTree);
	m_is_use_tree = SETTING(USE_SEARCH_GROUP_TREE_SETTINGS) != 0;
#endif
	setListViewExtStyle(ctrlResults, BOOLSETTING(SHOW_GRIDLINES), false);
	resultsContainer.SubclassWindow(ctrlResults.m_hWnd);
	m_Theme = GetWindowTheme(ctrlResults.m_hWnd);
	
	if (useSystemIcon)
	{
		ctrlResults.SetImageList(g_fileImage.getIconList(), LVSIL_SMALL);
	}
	else
	{
		ResourceLoader::LoadImageList(IDR_FOLDERS, images, 16, 16); // [~] Sergey Shushkanov
		ctrlResults.SetImageList(images, LVSIL_SMALL);
	}
	
	ctrlHubs.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_NOCOLUMNHEADER, WS_EX_CLIENTEDGE, IDC_HUB);
	setListViewExtStyle(ctrlHubs, BOOLSETTING(SHOW_GRIDLINES), true);
	hubsContainer.SubclassWindow(ctrlHubs.m_hWnd);
	
#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH
	ctrlGridFilters.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_REPORT | LVS_NOCOLUMNHEADER, WS_EX_CLIENTEDGE);
	ctrlGridFilters.SetExtendedGridStyle(PGS_EX_SINGLECLICKEDIT /*| PGS_EX_NOGRID*/ | PGS_EX_NOSHEETNAVIGATION);
	
	ctrlGridFilters.InsertColumn(FGC_INVERT, _T("Invert"), LVCFMT_CENTER, 14, 0);
	ctrlGridFilters.InsertColumn(FGC_TYPE, _T("Type"), LVCFMT_LEFT, 62, 0);
	ctrlGridFilters.InsertColumn(FGC_FILTER, _T("Filter"), LVCFMT_LEFT, 114, 0);
	ctrlGridFilters.InsertColumn(FGC_ADD, _T("Add"), LVCFMT_LEFT, 14, 0);
	ctrlGridFilters.InsertColumn(FGC_REMOVE, _T("Del"), LVCFMT_LEFT, 14, 0);
	
	ctrlGridFilters.SetBkColor(GetSysColor(COLOR_3DFACE));
	ctrlGridFilters.SetTextColor(GetSysColor(COLOR_BTNTEXT));
	
	insertIntofilter(ctrlGridFilters);
#endif
	ctrlFilter.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                  ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	                  
	ctrlFilterContainer.SubclassWindow(ctrlFilter.m_hWnd);
	ctrlFilter.SetFont(Fonts::g_systemFont);
	
	
	ctrlFilterSel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL |
	                     WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE);
	                     
	ctrlFilterSelContainer.SubclassWindow(ctrlFilterSel.m_hWnd);
	ctrlFilterSel.SetFont(Fonts::g_systemFont);
	
	searchLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	searchLabel.SetFont(Fonts::g_systemFont, FALSE);
	searchLabel.SetWindowText(CTSTRING(SEARCH_FOR));
	
	sizeLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	sizeLabel.SetFont(Fonts::g_systemFont, FALSE);
	sizeLabel.SetWindowText(CTSTRING(SIZE));
	
	typeLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	typeLabel.SetFont(Fonts::g_systemFont, FALSE);
	typeLabel.SetWindowText(CTSTRING(FILE_TYPE));
	
	srLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	srLabel.SetFont(Fonts::g_systemFont, FALSE);
	srLabel.SetWindowText(CTSTRING(SEARCH_IN_RESULTS));
	
#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH
	srLabelExcl.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	srLabelExcl.SetFont(Fonts::g_systemFont, FALSE);
	srLabelExcl.SetWindowText(CTSTRING(SEARCH_FILTER_LBL_EXCLUDE));
#endif
	
	optionLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	optionLabel.SetFont(Fonts::g_systemFont, FALSE);
	optionLabel.SetWindowText(CTSTRING(SEARCH_OPTIONS));
	if (!CompatibilityManager::isWine())
	{
		hubsLabel.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		hubsLabel.SetFont(Fonts::g_systemFont, FALSE);
		hubsLabel.SetWindowText(CTSTRING(HUBS));
	}
	ctrlSlots.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_FREESLOTS);
	ctrlSlots.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	ctrlSlots.SetFont(Fonts::g_systemFont, FALSE);
	ctrlSlots.SetWindowText(CTSTRING(ONLY_FREE_SLOTS));
	//slotsContainer.SubclassWindow(ctrlSlots.m_hWnd);
	
	ctrlCollapsed.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	ctrlCollapsed.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	ctrlCollapsed.SetFont(Fonts::g_systemFont, FALSE);
	ctrlCollapsed.SetWindowText(CTSTRING(EXPANDED_RESULTS));
	//collapsedContainer.SubclassWindow(ctrlCollapsed.m_hWnd);
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
	m_ctrlStoreIP.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	m_ctrlStoreIP.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	m_storeIP = BOOLSETTING(ENABLE_LAST_IP_AND_MESSAGE_COUNTER);
	m_ctrlStoreIP.SetCheck(m_storeIP);
	m_ctrlStoreIP.SetFont(Fonts::g_systemFont, FALSE);
	m_ctrlStoreIP.SetWindowText(CTSTRING(STORE_SEARCH_IP));
	//storeIPContainer.SubclassWindow(m_ctrlStoreIP.m_hWnd);
#endif
	ctrlStoreSettings.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	ctrlStoreSettings.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	if (BOOLSETTING(SAVE_SEARCH_SETTINGS))
	{
		ctrlStoreSettings.SetCheck(TRUE);
	}
	ctrlStoreSettings.SetFont(Fonts::g_systemFont, FALSE);
	ctrlStoreSettings.SetWindowText(CTSTRING(SAVE_SEARCH_SETTINGS_TEXT));
	//storeSettingsContainer.SubclassWindow(ctrlStoreSettings.m_hWnd);	
	
	m_tooltip.AddTool(ctrlStoreSettings, ResourceManager::SAVE_SEARCH_SETTINGS_TOOLTIP);
	if (BOOLSETTING(ONLY_FREE_SLOTS))
	{
		ctrlSlots.SetCheck(TRUE);
		onlyFree = true;
	}
	
	ctrlUseGroupTreeSettings.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	ctrlUseGroupTreeSettings.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	if (BOOLSETTING(USE_SEARCH_GROUP_TREE_SETTINGS))
	{
		ctrlUseGroupTreeSettings.SetCheck(TRUE);
	}
	ctrlUseGroupTreeSettings.SetFont(Fonts::g_systemFont, FALSE);
	ctrlUseGroupTreeSettings.SetWindowText(CTSTRING(USE_SEARCH_GROUP_TREE_SETTINGS_TEXT));
	
	ctrlUseTorrentSearch.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	ctrlUseTorrentSearch.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	if (BOOLSETTING(USE_TORRENT_SEARCH))
	{
		ctrlUseTorrentSearch.SetCheck(TRUE);
	}
	ctrlUseTorrentSearch.SetFont(Fonts::g_systemFont, FALSE);
	ctrlUseTorrentSearch.SetWindowText(CTSTRING(USE_TORRENT_SEARCH_TEXT));
	
	ctrlUseTorrentRSS.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_COLLAPSED);
	ctrlUseTorrentRSS.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	if (BOOLSETTING(USE_TORRENT_RSS))
	{
		ctrlUseTorrentRSS.SetCheck(TRUE);
	}
	ctrlUseTorrentRSS.SetFont(Fonts::g_systemFont, FALSE);
	ctrlUseTorrentRSS.SetWindowText(CTSTRING(USE_TORRENT_RSS_TEXT));
	
	ctrlShowUI.Create(ctrlStatus.m_hWnd, rcDefault, _T("+/-"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUI.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUI.SetCheck(1);
	ctrlShowUI.SetFont(Fonts::g_systemFont);
	showUIContainer.SubclassWindow(ctrlShowUI.m_hWnd);
	m_tooltip.AddTool(ctrlShowUI, ResourceManager::SEARCH_SHOWHIDEPANEL);
	
// ������ ������� �������. [<-] InfinitySky.
	ctrlPurge.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BS_ICON |
	                 BS_PUSHBUTTON, 0, IDC_PURGE);
	ctrlPurge.SetIcon(g_purge_icon); // [+] InfinitySky. ������ �� ������ ������� �������.
	//purgeContainer.SubclassWindow(ctrlPurge.m_hWnd);
	m_tooltip.AddTool(ctrlPurge, ResourceManager::CLEAR_SEARCH_HISTORY);
// ������ �����. [<-] InfinitySky.
	ctrlPauseSearch.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                       BS_PUSHBUTTON, 0, IDC_SEARCH_PAUSE);
	ctrlPauseSearch.SetWindowText(CTSTRING(PAUSE));
	ctrlPauseSearch.SetFont(Fonts::g_systemFont);
	ctrlPauseSearch.SetIcon(g_pause_icon); // [+] InfinitySky. ������ �� ������ �����.
	
// ������ ������. [<-] InfinitySky.
	ctrlDoSearch.Create(l_lHwnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	                    BS_PUSHBUTTON, 0, IDC_SEARCH);
	ctrlDoSearch.SetWindowText(CTSTRING(SEARCH));
	ctrlDoSearch.SetFont(Fonts::g_systemFont);
	ctrlDoSearch.SetIcon(g_search_icon); // [+] InfinitySky. ������ �� ������ ������.
	//doSearchContainer.SubclassWindow(ctrlDoSearch.m_hWnd);
	
	
	ctrlSearchBox.SetFont(Fonts::g_systemFont, FALSE);
	ctrlSize.SetFont(Fonts::g_systemFont, FALSE);
	ctrlMode.SetFont(Fonts::g_systemFont, FALSE);
	ctrlSizeMode.SetFont(Fonts::g_systemFont, FALSE);
	ctrlFiletype.SetFont(Fonts::g_systemFont, FALSE);
	
	ctrlMode.AddString(CTSTRING(ANY));
	ctrlMode.AddString(CTSTRING(AT_LEAST));
	ctrlMode.AddString(CTSTRING(AT_MOST));
	ctrlMode.AddString(CTSTRING(EXACT_SIZE));
	
	ctrlSizeMode.AddString(CTSTRING(B));
	ctrlSizeMode.AddString(CTSTRING(KB));
	ctrlSizeMode.AddString(CTSTRING(MB));
	ctrlSizeMode.AddString(CTSTRING(GB));
	
	ctrlFiletype.AddString(CTSTRING(ANY));
	ctrlFiletype.AddString(CTSTRING(AUDIO));
	ctrlFiletype.AddString(CTSTRING(COMPRESSED));
	ctrlFiletype.AddString(CTSTRING(DOCUMENT));
	ctrlFiletype.AddString(CTSTRING(EXECUTABLE));
	ctrlFiletype.AddString(CTSTRING(PICTURE));
	ctrlFiletype.AddString(CTSTRING(VIDEO_AND_SUBTITLES));
	ctrlFiletype.AddString(CTSTRING(DIRECTORY));
	ctrlFiletype.AddString(_T("TTH"));
	ctrlFiletype.AddString(CTSTRING(CD_DVD_IMAGES));
	ctrlFiletype.AddString(CTSTRING(COMICS));
	ctrlFiletype.AddString(CTSTRING(BOOK));
	if (BOOLSETTING(SAVE_SEARCH_SETTINGS))
	{
		ctrlFiletype.SetCurSel(SETTING(SAVED_SEARCH_TYPE));
		ctrlSizeMode.SetCurSel(SETTING(SAVED_SEARCH_SIZEMODE));
		ctrlMode.SetCurSel(SETTING(SAVED_SEARCH_MODE));
		SetDlgItemText(IDC_SEARCH_SIZE, Text::toT(SETTING(SAVED_SEARCH_SIZE)).c_str());
	}
	else
	{
		ctrlFiletype.SetCurSel(0);
		if (m_initialSize == 0)
			ctrlSizeMode.SetCurSel(2);
		else
			ctrlSizeMode.SetCurSel(0);
		ctrlMode.SetCurSel(1);
	}
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(SEARCH_FRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokensWidth(columnSizes, SETTING(SEARCH_FRAME_WIDTHS), COLUMN_LAST);
	
	BOOST_STATIC_ASSERT(_countof(columnSizes) == COLUMN_LAST);
	BOOST_STATIC_ASSERT(_countof(columnNames) == COLUMN_LAST);
	
	for (uint8_t j = 0; j < COLUMN_LAST; j++)
	{
		const int fmt = (j == COLUMN_SIZE || j == COLUMN_EXACT_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlResults.InsertColumn(j, TSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlResults.setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlResults.setVisible(SETTING(SEARCH_FRAME_VISIBLE));
	int sortColumn = SETTING(SEARCH_FRAME_SORT);
	if (!sortColumn || abs(sortColumn) > COLUMN_LAST)
		sortColumn = -(COLUMN_HITS + 1);
	ctrlResults.setSortFromSettings(sortColumn);
	
	setListViewColors(ctrlResults);
	ctrlResults.SetFont(Fonts::g_systemFont, FALSE); // use Util::font instead to obey Appearace settings
	ctrlResults.setFlickerFree(Colors::g_bgBrush);
	ctrlResults.setColumnOwnerDraw(COLUMN_LOCATION);
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
	ctrlResults.setColumnOwnerDraw(COLUMN_ANTIVIRUS);
	ctrlResults.SetColumnWidth(COLUMN_ANTIVIRUS, 0);
#endif
	ctrlResults.setColumnOwnerDraw(COLUMN_P2P_GUARD);
	
	ctrlHubs.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, LVSCW_AUTOSIZE, 0);
	setListViewColors(ctrlHubs);
	ctrlHubs.SetFont(Fonts::g_systemFont, FALSE); // use Util::font instead to obey Appearace settings
	
	copyMenu.CreatePopupMenu();
	copyMenuTorrent.CreatePopupMenu();
	targetDirMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	priorityMenu.CreatePopupMenu();
	tabMenu.CreatePopupMenu();  // [+] InfinitySky
	
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_ALL_SEARCH_FRAME, CTSTRING(MENU_CLOSE_ALL_SEARCHFRAME));
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE_HOT));
	
	// [-] brain-ripper
	// Make menu dynamic (in context menu handler), since its content depends of which
	// user selected (for add/remove favorites item)
	//resultsMenu.CreatePopupMenu();
	
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_FILENAME, CTSTRING(FILENAME));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_TTH, CTSTRING(TTH_ROOT_OR_TORRENT_MAGNET));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_SIZE, CTSTRING(SIZE));
	//copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_FULL_MAGNET_LINK, CTSTRING(COPY_FULL_MAGNET_LINK));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_TORRENT_DATE, CTSTRING(TORRENT_DATE));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_TORRENT_COMMENT, CTSTRING(TORRENT_COMMENT));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_TORRENT_URL, CTSTRING(TORRENT_URL));
	copyMenuTorrent.AppendMenu(MF_STRING, IDC_COPY_TORRENT_PAGE, CTSTRING(TORRENT_PAGE));
	
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CTSTRING(COPY_NICK));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_FILENAME, CTSTRING(FILENAME));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_PATH, CTSTRING(PATH));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_SIZE, CTSTRING(SIZE));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_HUB_URL, CTSTRING(HUB_ADDRESS));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_TTH, CTSTRING(TTH_ROOT_OR_TORRENT_MAGNET));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_LINK, CTSTRING(COPY_MAGNET_LINK));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_FULL_MAGNET_LINK, CTSTRING(COPY_FULL_MAGNET_LINK));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_WMLINK, CTSTRING(COPY_MLINK_TEMPL)); // !SMT!-UI
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_PAUSED, CTSTRING(PAUSED));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOWEST, CTSTRING(LOWEST));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOW, CTSTRING(LOW));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_NORMAL, CTSTRING(NORMAL));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGH, CTSTRING(HIGH));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGHEST, CTSTRING(HIGHEST));
	/*
	resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS, CTSTRING(DOWNLOAD));
	resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetMenu, CTSTRING(DOWNLOAD_TO));
	resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS, CTSTRING(DOWNLOAD_WHOLE_DIR));
	resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetDirMenu, CTSTRING(DOWNLOAD_WHOLE_DIR_TO));
	#ifdef FLYLINKDC_USE_VIEW_AS_TEXT_OPTION
	resultsMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	#endif
	resultsMenu.AppendMenu(MF_SEPARATOR);
	resultsMenu.AppendMenu(MF_STRING, IDC_MARK_AS_DOWNLOADED, CTSTRING(MARK_AS_DOWNLOADED));
	resultsMenu.AppendMenu(MF_SEPARATOR);
	resultsMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	
	resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenu, CTSTRING(COPY));
	resultsMenu.AppendMenu(MF_SEPARATOR);
	appendUserItems(resultsMenu, Util::emptyString); // TODO: hubhint
	resultsMenu.AppendMenu(MF_SEPARATOR);
	resultsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
	resultsMenu.SetMenuDefaultItem(IDC_DOWNLOAD_FAVORITE_DIRS);
	*/
	
	ctrlUDPMode.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_ICON | BS_CENTER | BS_PUSHBUTTON, 0);
	ctrlUDPTestResult.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlUDPTestResult.SetFont(Fonts::g_systemFont, FALSE);
	
	showPortStatus();
	
	UpdateLayout();
	for (int j = 0; j < COLUMN_LAST; j++)
	{
		ctrlFilterSel.AddString(CTSTRING_I(columnNames[j]));
	}
	ctrlFilterSel.SetCurSel(0);
	ctrlStatus.SetText(1, 0, SBT_OWNERDRAW);
	m_tooltip.SetMaxTipWidth(200);   //[+] SCALOlaz: activate tooltips
	if (!BOOLSETTING(POPUPS_DISABLED))
	{
		m_tooltip.Activate(TRUE);
	}
	onSizeMode();   //Get Mode, and turn ON or OFF controlls Size
	SearchManager::getInstance()->addListener(this);
	initHubs();
#ifdef FLYLINKDC_USE_TREE_SEARCH
	m_RootTreeItem = nullptr;
	m_RootVirusTreeItem = nullptr;
	m_RootTorrentRSSTreeItem = nullptr;
	m_24HTopTorrentTreeItem = nullptr;
	m_CurrentTreeItem = nullptr;
	m_OldTreeItem = nullptr;
	clear_tree_filter_contaners();
#endif
	if (!m_initialString.empty())
	{
		g_lastSearches.push_front(m_initialString);
		ctrlSearchBox.InsertString(0, m_initialString.c_str());
		ctrlSearchBox.SetCurSel(0);
		ctrlMode.SetCurSel(m_initialMode);
		ctrlSize.SetWindowText(Util::toStringW(m_initialSize).c_str());
		ctrlFiletype.SetCurSel(m_initialType);
		
		onEnter();
	}
	else
	{
		SetWindowText(CTSTRING(SEARCH));
		::EnableWindow(GetDlgItem(IDC_SEARCH_PAUSE), FALSE);
		m_running = false;
	}
	SettingsManager::getInstance()->addListener(this);
	create_timer(1000);
	
	if (m_is_disable_torrent_RSS == false && BOOLSETTING(USE_TORRENT_RSS))
	{
		m_torrentRSSThread.start_torrent_top(m_win_handler);
	}
	
	bHandled = FALSE;
	return 1;
}

#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH
vector<LPCTSTR> SearchFrame::getFilterTypes()
{
	vector<LPCTSTR> types;
	
	// default types
	for (int j = 0; j < COLUMN_LAST; j++)
	{
		types.push_back(CTSTRING_I(columnNames[j]));
	}
	
	// addition
	for (int j = FILTER_ITEM_FIRST; j < FILTER_ITEM_LAST; j++)
	{
		LPCTSTR caption = _T("");
		switch (j)
		{
			case FILTER_ITEM_PATHFILE:
			{
				caption = CTSTRING(SEARCH_FILTER_ITEM_FILEPATH);
				break;
			}
			default:
			{
				ATLASSERT(false);
			}
		}
		types.push_back(caption);
	}
	return types;
}

void SearchFrame::insertIntofilter(CGrid& grid)
{
	TCGridItemCustom invertCfg;
	invertCfg.caption     = _T("~");
	invertCfg.flags       = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
	invertCfg.offset.top  = 4;
	
	int idx = grid.InsertItem(grid.GetSelectedIndex() + 1, CGridItem::ButtonPush(false, invertCfg));
	
	TCGridItemCustom typesCfg;
	typesCfg.offset.right = 70;
	
	grid.SetSubItem(idx, 1, CGridItem::ListBox(getFilterTypes(), typesCfg, FILTER_ITEM_PATHFILE));
	grid.SetSubItem(idx, 2, CGridItem::EditBox(_T(""), this));
	grid.SetSubItem(idx, 3, CGridItem::Button(_T("+")));
	grid.SetSubItem(idx, 4, CGridItem::Button(_T("-")));
}
#endif

LRESULT SearchFrame::onMeasure(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd = 0;
	if (wParam == IDC_FILETYPES)
		return ListMeasure(hwnd, wParam, (MEASUREITEMSTRUCT *)lParam);
	else
		return OMenu::onMeasureItem(hwnd, uMsg, wParam, lParam, bHandled);
}

LRESULT SearchFrame::onDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd = 0;
	const DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
	bHandled = FALSE;
	
	if (wParam == IDC_FILETYPES)
	{
		return ListDraw(hwnd, wParam, (DRAWITEMSTRUCT*)lParam);
	}
	else if (dis->CtlID == ATL_IDW_STATUS_BAR && dis->itemID == 1)
	{
		const auto l_delta = m_searchEndTime - m_searchStartTime;
		if (m_searchStartTime > 0 && l_delta)
		{
			bHandled = TRUE;
			const RECT rc = dis->rcItem;
			int borders[3];
			
			ctrlStatus.GetBorders(borders);
			
			CDC dc(dis->hDC);
			
			const uint64_t now = GET_TICK();
			const uint64_t length = min((uint64_t)(rc.right - rc.left), (rc.right - rc.left) * (now - m_searchStartTime) / l_delta);
			
			OperaColors::FloodFill(dc, rc.left, rc.top, rc.left + (LONG)length, rc.bottom, RGB(128, 128, 128), RGB(160, 160, 160));
			
			dc.SetBkMode(TRANSPARENT);
			const int textHeight = WinUtil::getTextHeight(dc);
			const LONG top = rc.top + (rc.bottom - rc.top - textHeight) / 2;
			
			dc.SetTextColor(RGB(255, 255, 255));
			RECT rc2 = rc;
			rc2.right = rc.left + (LONG)length;
			dc.ExtTextOut(rc.left + borders[2], top, ETO_CLIPPED, &rc2, statusLine.c_str(), statusLine.size(), NULL);
			
			dc.SetTextColor(Colors::g_textColor);
			rc2 = rc;
			rc2.left = rc.left + (LONG)length;
			dc.ExtTextOut(rc.left + borders[2], top, ETO_CLIPPED, &rc2, statusLine.c_str(), statusLine.size(), NULL);
			
			dc.Detach();
		}
	}
	else if (dis->CtlType == ODT_MENU)
	{
		bHandled = TRUE;
		return OMenu::onDrawItem(hwnd, uMsg, wParam, lParam, bHandled);
	}
	
	return S_OK;
}

BOOL SearchFrame::ListMeasure(HWND /*hwnd*/, UINT /*uCtrlId*/, MEASUREITEMSTRUCT *mis)
{
	mis->itemHeight = 16;
	return TRUE;
}

BOOL SearchFrame::ListDraw(HWND /*hwnd*/, UINT /*uCtrlId*/, DRAWITEMSTRUCT *dis)
{
	TCHAR szText[MAX_PATH];
	szText[0] = 0;
	
	switch (dis->itemAction)
	{
		case ODA_FOCUS:
			if (!(dis->itemState & ODS_NOFOCUSRECT))
				DrawFocusRect(dis->hDC, &dis->rcItem);
			break;
			
		case ODA_SELECT:
		case ODA_DRAWENTIRE:
			ctrlFiletype.GetLBText(dis->itemID, szText);
			if (dis->itemState & ODS_SELECTED)
			{
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHT));
			}
			else
			{
				SetTextColor(dis->hDC, Colors::g_textColor);
				SetBkColor(dis->hDC, Colors::g_bgColor);
			}
			
			ExtTextOut(dis->hDC, dis->rcItem.left + 22, dis->rcItem.top + 1, ETO_OPAQUE, &dis->rcItem, szText, wcslen(szText), 0);
			if (dis->itemState & ODS_FOCUS)
			{
				if (!(dis->itemState & ODS_NOFOCUSRECT))
					DrawFocusRect(dis->hDC, &dis->rcItem);
			}
			
			ImageList_Draw(m_searchTypesImageList, dis->itemID, dis->hDC,
			               dis->rcItem.left + 2,
			               dis->rcItem.top,
			               ILD_TRANSPARENT);
			break;
	}
	return TRUE;
}

void SearchFrame::init_last_search_box()
{
	while (ctrlSearchBox.GetCount())
	{
		ctrlSearchBox.DeleteString(0);
	}
	for (auto i = g_lastSearches.cbegin(); i != g_lastSearches.cend(); ++i)
	{
		ctrlSearchBox.AddString(i->c_str());
	}
}

int SearchFrame::TorrentTopSender::run()
{
#if 0
	try
	{
		CFlyServerConfig::torrentGetTop(m_wnd, PREPARE_RESULT_TOP_TORRENT);
	}
	catch (const std::bad_alloc&)
	{
		ShareManager::tryFixBadAlloc();
	}
#endif
	return 0;
}

int SearchFrame::TorrentSearchSender::run()
{
#if 0
	try
	{
		dcassert(!search.empty());
		if (!search.empty())
		{
			CFlyServerConfig::torrentSearch(m_wnd, PREPARE_RESULT_TORRENT, search);
		}
	}
	catch (const std::bad_alloc&)
	{
		ShareManager::tryFixBadAlloc();
	}
#endif
	return 0;
}

void SearchFrame::onEnter()
{
	BOOL tmp_Handled;
	onEditChange(0, 0, NULL, tmp_Handled); // if in searchbox TTH - select filetypeTTH
	
	CFlyBusyBool l_busy(m_is_before_search);
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
	clearFlyServerQueue();
#endif
	searchParam.clients.clear();
	
	ctrlResults.DeleteAndClearAllItems();
#ifdef FLYLINKDC_USE_TREE_SEARCH
	clear_tree_filter_contaners();
#endif
	
	tstring s;
	WinUtil::getWindowText(ctrlSearch, s);
	
	// Add new searches to the last-search dropdown list
	if (!BOOLSETTING(FORGET_SEARCH_REQUEST))
	{
		g_lastSearches.remove(s);
		const int i = max(SETTING(SEARCH_HISTORY) - 1, 0);
		
		while (g_lastSearches.size() > TStringList::size_type(i))
		{
			g_lastSearches.pop_back();
		}
		g_lastSearches.push_front(s);
		init_last_search_box();
		CFlylinkDBManager::getInstance()->clean_registry(e_SearchHistory, 0);
		CFlylinkDBManager::getInstance()->save_registry(g_lastSearches, e_SearchHistory); //[+]PPA
	}
	MainFrame::updateQuickSearches();
	if (BOOLSETTING(USE_TORRENT_SEARCH) && searchParam.fileType != FILE_TYPE_TTH && !isTTH(s) && !s.empty())
	{
		m_torrentSearchThread.start_torrent_search(m_win_handler, s);
	}
	// Change Default Settings If Changed
	if (onlyFree != BOOLSETTING(ONLY_FREE_SLOTS))
	{
		SET_SETTING(ONLY_FREE_SLOTS, onlyFree);
	}
	const int n = ctrlHubs.GetItemCount();
	if (!CompatibilityManager::isWine())
	{
		for (int i = 0; i < n; i++)
		{
			if (ctrlHubs.GetCheckState(i))
			{
				const tstring& url = ctrlHubs.getItemData(i)->url;
				searchParam.clients.push_back(Text::fromT(url));
			}
		}
		if (searchParam.clients.empty())
			return;
	}
	
	tstring sizeStr;
	WinUtil::getWindowText(ctrlSize, sizeStr);
	
	double size = Util::toDouble(Text::fromT(sizeStr));
	switch (ctrlSizeMode.GetCurSel())
	{
		case 1:
			size *= 1024.0;
			break;
		case 2:
			size *= 1024.0 * 1024.0;
			break;
		case 3:
			size *= 1024.0 * 1024.0 * 1024.0;
			break;
	}
	
	searchParam.size = size;
	
	clearPausedResults();
	
	::EnableWindow(GetDlgItem(IDC_SEARCH_PAUSE), TRUE);
	ctrlPauseSearch.SetWindowText(CTSTRING(PAUSE));
	
	search = StringTokenizer<string>(Text::fromT(s), ' ').getTokens(); // [~]IRainman optimize SearchFrame
	searchParam.fileType = ctrlFiletype.GetCurSel();
	
	s.clear();
	{
		CFlyFastLock(m_fcs);
		//strip out terms beginning with -
		for (auto si = search.cbegin(); si != search.cend();)
		{
			if (si->empty())
			{
				si = search.erase(si);
				continue;
			}
			if (searchParam.fileType == FILE_TYPE_TTH)
			{
				if (!isTTH(Text::toT(*si)))
				{
					LogManager::message("[Search] Error TTH format = " + *si);
					searchParam.fileType = FILE_TYPE_ANY;
					ctrlFiletype.SetCurSel(0);
				}
			}
			if ((*si)[0] != '-')
				s += Text::toT(*si) + _T(' ');
			++si;
		}
		searchParam.token = Util::rand();
	}
	s = s.substr(0, max(s.size(), static_cast<tstring::size_type>(1)) - 1);
	
	if (s.empty())
		return;
		
	m_target = s;
	
	if (searchParam.size == 0)
		searchParam.sizeMode = Search::SIZE_DONTCARE;
	else
		searchParam.sizeMode = Search::SizeModes(ctrlMode.GetCurSel());
		
	ctrlStatus.SetText(3, _T(""));
	ctrlStatus.SetText(4, _T(""));
	
	m_isExactSize = searchParam.sizeMode == Search::SIZE_EXACT;
	m_exactSize2 = searchParam.size;
	
	if (BOOLSETTING(CLEAR_SEARCH))
	{
		ctrlSearch.SetWindowText(_T(""));
	}
	
	droppedResults = 0;
	resultsCount = 0;
	m_running = true;
	m_isHash = (searchParam.fileType == FILE_TYPE_TTH);
	
	clearPausedResults();
	
	SetWindowText((TSTRING(SEARCH) + _T(" - ") + s).c_str());
	
	// [+] merge
	// stop old search
	ClientManager::cancelSearch((void*)this);
	searchParam.extList.clear();
	// Get ADC searchtype extensions if any is selected
	try
	{
		if (searchParam.fileType == FILE_TYPE_ANY)
		{
			// Custom searchtype
			// disabled with current GUI extList = SettingsManager::getInstance()->getExtensions(Text::fromT(fileType->getText()));
		}
		else if ((searchParam.fileType > FILE_TYPE_ANY && searchParam.fileType < FILE_TYPE_DIRECTORY) /* TODO - || m_ftype == FILE_TYPE_CD_DVD*/)
		{
			// Predefined searchtype
			searchParam.extList = SettingsManager::getExtensions(string(1, '0' + searchParam.fileType));
		}
	}
	catch (const SearchTypeException&)
	{
		searchParam.fileType = FILE_TYPE_ANY;
	}
	
	{
		CFlyFastLock(m_fcs);
		
		m_searchStartTime = GET_TICK();
		// more 10 seconds for transfering results
		searchParam.filter = Text::fromT(s);
		searchParam.normalizeWhitespace();
		searchParam.owner = this;
		if (portStatus == PortTest::STATE_FAILURE) // || (SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5);
			searchParam.forcePassive = true;
		else
			searchParam.forcePassive = false;
		m_searchEndTime = m_searchStartTime + ClientManager::multiSearch(searchParam) + 10000;
		m_waitingResults = true;
	}
	
	::EnableWindow(GetDlgItem(IDC_SEARCH_PAUSE), TRUE);
	ctrlPauseSearch.SetWindowText(CTSTRING(PAUSE));
}

void SearchFrame::removeSelected()
{
	int i = -1;
	CFlyFastLock(m_fcs);
	while ((i = ctrlResults.GetNextItem(-1, LVNI_SELECTED)) != -1)
	{
		ctrlResults.removeGroupedItem(ctrlResults.getItemData(i));
		// TODO - delete from set
	}
}

void SearchFrame::on(SearchManagerListener::SR, const SearchResult& sr) noexcept
{
	if (isClosedOrShutdown())
		return;
	// Check that this is really a relevant search result...
	{
		CFlyFastLock(m_fcs);
		
		if (search.empty())
			return;
			
		needUpdateResultCount = true;
		if (sr.getToken() && searchParam.token != sr.getToken())
		{
			droppedResults++;
			return;
		}
		
		string l_ext;
		bool l_is_executable = false;
		if (sr.getType() == SearchResult::TYPE_FILE)
		{
			l_ext = "x" + Util::getFileExt(sr.getFileName());
			l_is_executable = ShareManager::checkType(l_ext, FILE_TYPE_EXECUTABLE) && sr.getSize();
		}
		if (m_isHash)
		{
			if (sr.getType() != SearchResult::TYPE_FILE || TTHValue(search[0]) != sr.getTTH())
			{
				droppedResults++;
				return;
			}
#if 0
			// Level 4
			// ��� �� ��������� ����� - aResult.getUser()->getLastNick()
			size_t l_count = check_antivirus_level(make_pair(sr.getTTH(),  sr.getHubName() + " ( " + sr.getHubUrl() + " ) "), *aResult, 4);
			if (l_count > CFlyServerConfig::g_unique_files_for_virus_detect && l_is_executable)
			{
				const int l_virus_level = 4;
				// TODO CFlyServerConfig::addBlockIP(aResult.getIPAsString());
				if (registerVirusLevel(*aResult, l_virus_level))
				{
					CFlyServerJSON::addAntivirusCounter(*aResult, l_count, l_virus_level);
				}
			}
			
#endif
		}
		else
		{
#if 0
			if (m_search_param.fileType != FILE_TYPE_EXECUTABLE && m_search_param.fileType != Search::TYPE_ANY && m_search_param.fileType != FILE_TYPE_DIRECTORY)
			{
				if (l_is_executable)
				{
					const int l_virus_level = 3;
					CFlyServerJSON::addAntivirusCounter(*aResult, 0, l_virus_level);
					sr.m_virus_level = l_virus_level;
					LogManager::virus_message("Search: ignore virus result  (Level 3): TTH = " + sr.getTTH().toBase32() +
					                          " File: " + sr.getFileName() + +" Size:" + Util::toString(sr.getSize()) +
					                          " Hub: " + sr.getHubUrl() + " Nick: " + sr.getUser()->getLastNick() + " IP = " + sr.getIPAsString());
					// http://dchublist.ru/forum/viewtopic.php?p=22426#p22426
					droppedResults++;
					return;
				}
			}
#endif			
#if 0
			if (l_is_executable || ShareManager::checkType(l_ext, FILE_TYPE_COMPRESSED))
			{
				// Level 1
				size_t l_count = check_antivirus_level(make_pair(sr.getTTH(), sr.getUser()->getLastNick() + " Hub:" + sr.getHubName() + " ( " + sr.getHubUrl() + " ) "), *aResult, 1);
				if (l_count > CFlyServerConfig::g_unique_files_for_virus_detect && l_is_executable) // 100$ Virus - block IP ?
				{
					const int l_virus_level = 1;
					// TODO CFlyServerConfig::addBlockIP(aResult.getIPAsString());
					if (registerVirusLevel(*aResult, l_virus_level))
					{
						CFlyServerJSON::addAntivirusCounter(*aResult, l_count, l_virus_level);
					}
				}
				// TODO - �������� ���������� ��������� ������ return;
				// Level 2
				l_count = check_antivirus_level(make_pair(sr.getTTH(), "."), *aResult, 2);
				if (l_count)
				{
					const int l_virus_level = 2;
					// TODO - �������� ���������� ��������� ������ return;
				}
			}
#endif
			// https://github.com/pavel-pimenov/flylinkdc-r5xx/issues/18
			const int l_local_filter[] =
			{
				FILE_TYPE_AUDIO,
				FILE_TYPE_COMPRESSED,
				FILE_TYPE_DOCUMENT,
				FILE_TYPE_EXECUTABLE,
				FILE_TYPE_IMAGE,
				FILE_TYPE_VIDEO,
				FILE_TYPE_CD_DVD,
				FILE_TYPE_COMICS,
				FILE_TYPE_EBOOK,
			};
			for (auto k = 0; k < _countof(l_local_filter); ++k)
			{
				if (searchParam.fileType == l_local_filter[k])
				{
					const bool l_is_filter = ShareManager::checkType(l_ext, l_local_filter[k]);
					if (!l_is_filter)
					{
						droppedResults++;
						return;
					}
				}
			}
			// match all here
			tstring fileName = Text::toT(sr.getFile());
			Text::makeLower(fileName);
			for (auto j = search.cbegin(); j != search.cend(); ++j)
			{
				if (j->empty()) continue;
				tstring search = Text::toT(*j);
				Text::makeLower(search);
				if (search[0] != '-')
				{
					if (fileName.find(search) == tstring::npos)
					{
						droppedResults++;
						return;
					}
				}
				else if (search.length() > 1)
				{
					if (fileName.find(search.substr(1)) != tstring::npos)
					{
						droppedResults++;
						return;
					}
				}
			}
		}
	}
	// Reject results without free slots or size
	if ((onlyFree && sr.freeSlots < 1) || (m_isExactSize && sr.getSize() != m_exactSize2))
	{
		droppedResults++;
		//PostMessage(WM_SPEAKER, FILTER_RESULT);//[-]IRainman optimize SearchFrame
		return;
	}
	auto searchInfo = new SearchInfo(sr);
	{
		CFlyFastLock(m_si_set_cs);
		m_si_set.insert(searchInfo);
	}
	if (isClosedOrShutdown())
		return;
	if (!safe_post_message(*this, ADD_RESULT, searchInfo))
	{
		CFlyFastLock(m_si_set_cs);
		m_si_set.erase(searchInfo);
		dcassert(0);
	}
}

LRESULT SearchFrame::onCollapsed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_expandSR = ctrlCollapsed.GetCheck() == 1;
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
	m_storeIP = m_ctrlStoreIP.GetCheck() == 1;
#endif
	m_storeSettings = ctrlStoreSettings.GetCheck() == 1;
	SET_SETTING(SAVE_SEARCH_SETTINGS, m_storeSettings);
	m_is_use_tree = ctrlUseGroupTreeSettings.GetCheck() == 1;
	SET_SETTING(USE_SEARCH_GROUP_TREE_SETTINGS, m_is_use_tree);
	SET_SETTING(USE_TORRENT_SEARCH, ctrlUseTorrentSearch.GetCheck() == 1);
	SET_SETTING(USE_TORRENT_RSS, ctrlUseTorrentRSS.GetCheck() == 1);
	UpdateLayout();
	if (!m_is_use_tree)
	{
		if (m_RootTreeItem)
		{
			m_OldTreeItem = m_CurrentTreeItem;
			m_ctrlSearchFilterTree.SelectItem(m_RootTreeItem);
		}
	}
	else
	{
		if (m_OldTreeItem)
		{
			if (m_ctrlSearchFilterTree.SelectItem(m_OldTreeItem))
			{
				m_CurrentTreeItem = m_OldTreeItem;
			}
			else
			{
				dcassert(0);
			}
		}
	}
	return 0;
}

LRESULT SearchFrame::onTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (!isClosedOrShutdown())
	{
		showPortStatus();
		if (!MainFrame::isAppMinimized(m_hWnd)) // [+] IRainman opt.
		{
			static int g_index = 0;
			if (m_RootVirusTreeItem)
			{
				if (++g_index >= 3)
					g_index = 0;
				m_ctrlSearchFilterTree.SetItemImage(m_RootVirusTreeItem, m_skull_index + g_index, m_skull_index + g_index);
			}
			if (needUpdateResultCount)
				updateResultCount();
			if (m_need_resort)
			{
				m_need_resort = false;
				ctrlResults.resort();
			}
		}
		if (m_waitingResults)
		{
			uint64_t tick = GET_TICK();
			updateStatusLine(tick);
			m_waitingResults = tick < m_searchEndTime + 5000;
		}
	}
	return 0;
}

int SearchFrame::SearchInfo::compareItems(const SearchInfo* a, const SearchInfo* b, int col)
{
	switch (col)
	{
		case COLUMN_TYPE:
			if (a->m_sr.getType() == b->m_sr.getType())
				return lstrcmpi(a->getText(COLUMN_TYPE).c_str(), b->getText(COLUMN_TYPE).c_str());
			else
				return (a->m_sr.getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
		case COLUMN_HITS:
			if (!a->m_is_torrent)
				return compare(a->hits, b->hits);
			else
				return compare(a->m_sr.m_peer * 1000 + a->m_sr.m_seed, b->m_sr.m_peer * 1000 + b->m_sr.m_seed);
		case COLUMN_SLOTS:
			if (a->m_sr.freeSlots == b->m_sr.freeSlots)
				return compare(a->m_sr.slots, b->m_sr.slots);
			else
				return compare(a->m_sr.freeSlots, b->m_sr.freeSlots);
		case COLUMN_SIZE:
		case COLUMN_EXACT_SIZE:
			if (a->m_sr.getType() == b->m_sr.getType())
				return compare(a->m_sr.getSize(), b->m_sr.getSize());
			else
				return (a->m_sr.getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
		case COLUMN_FLY_SERVER_RATING: // TODO - ���������� x/y
		case COLUMN_BITRATE:
			return compare(Util::toInt64(a->columns[col]), Util::toInt64(b->columns[col]));
		case COLUMN_MEDIA_XY:
		{
			// TODO: please opt this! many call of getText, and no needs substr here!
			const auto a_pos = a->getText(col).find('x');
			if (a_pos != string::npos)
			{
				const auto b_pos = b->getText(col).find('x');
				if (b_pos != string::npos)
					return compare(Util::toInt(a->getText(col).substr(0, a_pos)) * 1000000 + Util::toInt(a->getText(col).substr(a_pos + 1, string::npos)), Util::toInt(b->getText(col).substr(0, b_pos)) * 1000000 + Util::toInt(b->getText(col).substr(b_pos + 1, string::npos)));
			}
			return compare(a->getText(col), b->getText(col));
			// ~TODO: please opt this! many call of getText, and no needs substr here!
		}
		case COLUMN_IP:
			return compare(Socket::convertIP4(Text::fromT(a->getText(COLUMN_IP))), Socket::convertIP4(Text::fromT(b->getText(COLUMN_IP))));
		case COLUMN_TORRENT_COMMENT:
			return compare(a->m_sr.m_comment, b->m_sr.m_comment);
		default:
			return Util::defaultSort(a->getText(col), b->getText(col));
	}
}

void SearchFrame::SearchInfo::calcImageIndex()
{
	if (m_is_torrent)
	{
		m_icon_index = FileImage::DIR_TORRENT;
	}
	else
	{
		if (m_icon_index < 0)
		{
			if (m_sr.getType() == SearchResult::TYPE_FILE)
				m_icon_index = g_fileImage.getIconIndex(m_sr.getFile());
			else
				m_icon_index = FileImage::DIR_ICON;
		}
	}
}

int SearchFrame::SearchInfo::getImageIndex() const
{
	//dcassert(m_icon_index >= 0);
	return m_icon_index;
}

const tstring SearchFrame::SearchInfo::getText(uint8_t col) const
{
	dcassert(col < COLUMN_LAST);
	if (m_is_torrent)
	{
		switch (col)
		{
			case COLUMN_TTH:
				//return Text::toT("SHA1:" + m_sr.getSHA1());
				return  Text::toT(m_sr.getTorrentMagnet());
			case COLUMN_FILENAME:
				return Text::toT(m_sr.getFile());
			case COLUMN_HITS:
				return Text::toT(m_sr.getPeersString());
			//case COLUMN_EXACT_SIZE:
			//  return m_sr.getSize() > 0 ? Util::formatExactSize(m_sr.getSize()) : Util::emptyStringT;
			case COLUMN_SIZE:
				return m_sr.getSize() > 0 ? Util::formatBytesW(m_sr.getSize()) : Util::emptyStringT;
			case COLUMN_TORRENT_COMMENT:
				return Text::toT(Util::toString(m_sr.m_comment));
			case COLUMN_TORRENT_DATE:
				return Text::toT(m_sr.m_date);
			case COLUMN_TORRENT_URL:
				return Text::toT(m_sr.m_url);
			case COLUMN_TORRENT_TRACKER:
				return Text::toT(m_sr.m_tracker);
			case COLUMN_TORRENT_PAGE:
				return Text::toT(Util::toString(m_sr.m_torrent_page));
		}
		return Util::emptyStringT;
	}
	else
	{
		switch (col)
		{
			case COLUMN_FILENAME:
				if (m_sr.getType() == SearchResult::TYPE_FILE)
				{
					if (m_sr.getFile().rfind(_T('\\')) == tstring::npos)
					{
						return Text::toT(m_sr.getFile());
					}
					else
					{
						return Text::toT(Util::getFileName(m_sr.getFile()));
					}
				}
				else
				{
					return Text::toT(m_sr.getFileName());
				}
			case COLUMN_HITS:
				return hits == 0 ? Util::emptyStringT : Util::toStringW(hits + 1) + _T(' ') + TSTRING(USERS);
			case COLUMN_NICK:
				if (getUser())
				{
					return Text::toT(Util::toString(ClientManager::getNicks(getUser()->getCID(), m_sr.getHubUrl(), false)));
				}
				else
				{
					return Util::emptyStringT;
				}
			// TODO - ��������� ��� � columns � ���������� ��� �� ����?
			case COLUMN_TYPE:
				if (m_sr.getType() == SearchResult::TYPE_FILE)
				{
					const tstring type = Text::toT(Util::getFileExtWithoutDot(Text::fromT(getText(COLUMN_FILENAME))));
					return type;
				}
				else
				{
					return TSTRING(DIRECTORY);
				}
			case COLUMN_EXACT_SIZE:
				return m_sr.getSize() > 0 ? Util::formatExactSize(m_sr.getSize()) : Util::emptyStringT;
			case COLUMN_SIZE:
				if (m_sr.getType() == SearchResult::TYPE_FILE)
				{
					return Util::formatBytesW(m_sr.getSize());
				}
				else
				{
					return Util::emptyStringT;
				}
			case COLUMN_PATH:
				if (m_sr.getType() == SearchResult::TYPE_FILE)
				{
					return Text::toT(Util::getFilePath(m_sr.getFile()));
				}
				else
				{
					return Text::toT(m_sr.getFile());
				}
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
			case COLUMN_ANTIVIRUS:
			{
				if (getUser())
					return Text::toT(Util::toString(ClientManager::getAntivirusNicks(getUser()->getCID())));
				else
					return "";
			}
#endif
			case COLUMN_LOCAL_PATH:
				return m_sr.getType() == SearchResult::TYPE_FILE? 
					ShareManager::toRealPathSafe(m_sr.getTTH()) : tstring();
			case COLUMN_SLOTS:
				return Util::toStringW(m_sr.freeSlots) + L'/' + Util::toStringW(m_sr.slots);
			// [-] PPA
			//case COLUMN_CONNECTION: return Text::toT(ClientManager::getInstance()->getConnection(getUser()->getCID()));
			case COLUMN_HUB:
				return Text::toT(m_sr.getHubName() + " (" + m_sr.getHubUrl() + ')');
			case COLUMN_IP:
				return Text::toT(m_sr.getIPAsString());
			case COLUMN_P2P_GUARD:
				return Text::toT(m_sr.getP2PGuard());
			case COLUMN_TTH:
				return m_sr.getType() == SearchResult::TYPE_FILE ? Text::toT(m_sr.getTTH().toBase32()) : Util::emptyStringT;
			case COLUMN_LOCATION:
				return Util::emptyStringT; // ������� �������� - �������� �� ownerDraw
			default:
			{
				if (col < COLUMN_LAST)
				{
					return columns[col];
				}
				else
				{
					return Util::emptyStringT;
				}
			}
		}
	}
	return Util::emptyStringT;
}

void SearchFrame::SearchInfo::view()
{
	try
	{
		if (m_sr.getType() == SearchResult::TYPE_FILE)
		{
			QueueManager::getInstance()->add(Util::getTempPath() + m_sr.getFileName(),
			                                 m_sr.getSize(), m_sr.getTTH(), m_sr.getHintedUser(),
			                                 QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_TEXT);
		}
	}
	catch (const Exception& e)
	{
		LogManager::message("Error SearchFrame::SearchInfo::view = " + e.getError());
	}
}

void SearchFrame::SearchInfo::Download::operator()(const SearchInfo* si)
{
	if (si->m_is_torrent == true)
	{
		dcassert(0);
		return;
	}
	try
	{
		if (prio == QueueItem::DEFAULT && WinUtil::isShift())
			prio = QueueItem::HIGHEST;
			
		if (si->m_sr.getType() == SearchResult::TYPE_FILE)
		{
			const string target = Text::fromT(m_tgt + si->getText(COLUMN_FILENAME));
			QueueManager::getInstance()->add(target, si->m_sr.getSize(), si->m_sr.getTTH(), si->m_sr.getHintedUser(), mask);
			
			const auto l_children = m_sf->getUserList().findChildren(si->getGroupCond()); // ������ ������ ������
			for (auto i = l_children.cbegin(); i != l_children.cend(); ++i)  // ��� ������ ������ ������������
			{
				const SearchInfo* j = *i;
				try
				{
					if (j)  // crash https://crash-server.com/Problem.aspx?ClientID=guest&ProblemID=44625
					{
						QueueManager::getInstance()->add(target, j->m_sr.getSize(), j->m_sr.getTTH(), j->m_sr.getHintedUser(), mask);
					}
				}
				catch (const Exception& e)
				{
					LogManager::message("QueueManager::getInstance()->add Error = " + e.getError());
				}
				
			}
			if (prio != QueueItem::DEFAULT)
				QueueManager::getInstance()->setPriority(target, prio);
		}
		else
		{
			QueueManager::getInstance()->addDirectory(si->m_sr.getFile(), si->m_sr.getHintedUser(), Text::fromT(m_tgt), prio);
		}
	}
	catch (const Exception& e)
	{
		LogManager::message("SearchInfo::Download Error = " + e.getError());
	}
	
}

void SearchFrame::SearchInfo::DownloadWhole::operator()(const SearchInfo* si)
{
	try
	{
		QueueItem::Priority prio = WinUtil::isShift() ? QueueItem::HIGHEST : QueueItem::DEFAULT;
		if (si->m_sr.getType() == SearchResult::TYPE_FILE)
		{
			QueueManager::getInstance()->addDirectory(Text::fromT(si->getText(COLUMN_PATH)), si->m_sr.getHintedUser(), Text::fromT(m_tgt), prio);
		}
		else
		{
			QueueManager::getInstance()->addDirectory(si->m_sr.getFile(), si->m_sr.getHintedUser(), Text::fromT(m_tgt), prio);
		}
	}
	catch (const Exception&)
	{
	}
}

void SearchFrame::SearchInfo::DownloadTarget::operator()(const SearchInfo* si)
{
	try
	{
		if (si->m_sr.getType() == SearchResult::TYPE_FILE)
		{
			const string target = Text::fromT(m_tgt);
			QueueManager::getInstance()->add(target, si->m_sr.getSize(), si->m_sr.getTTH(), si->m_sr.getHintedUser());
			
			if (WinUtil::isShift())
				QueueManager::getInstance()->setPriority(target, QueueItem::HIGHEST);
		}
		else
		{
			QueueManager::getInstance()->addDirectory(si->m_sr.getFile(), si->m_sr.getHintedUser(), Text::fromT(m_tgt),
			                                          WinUtil::isShift() ? QueueItem::HIGHEST : QueueItem::DEFAULT);
		}
	}
	catch (const Exception& e)
	{
		LogManager::message("SearchFrame::SearchInfo::DownloadTarget Error = " + e.getError());
	}
}

void SearchFrame::SearchInfo::getList()
{
	try
	{
		QueueManager::getInstance()->addList(m_sr.getHintedUser(), QueueItem::FLAG_CLIENT_VIEW, Text::fromT(getText(COLUMN_PATH)));
	}
	catch (const Exception&)
	{
		dcassert(0);
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::browseList()
{
	try
	{
		QueueManager::getInstance()->addList(m_sr.getHintedUser(), QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST, Text::fromT(getText(COLUMN_PATH)));
	}
	catch (const Exception&)
	{
		dcassert(0);
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::CheckTTH::operator()(const SearchInfo* si)
{
	if (firstTTH)
	{
		tth = si->getText(COLUMN_TTH);
		hasTTH = true;
		firstTTH = false;
	}
	else if (hasTTH)
	{
		if (tth != si->getText(COLUMN_TTH))
		{
			hasTTH = false;
		}
	}
	CID l_cid;
	if (si->m_sr.getUser())
		l_cid = si->m_sr.getUser()->getCID();
	if (firstHubs && hubs.empty() && si->m_sr.getUser())
	{
		hubs = ClientManager::getHubs(l_cid, si->m_sr.getHubUrl());
		firstHubs = false;
	}
	else if (!hubs.empty())
	{
		// we will merge hubs of all users to ensure we can use OP commands in all hubs
		const StringList sl = ClientManager::getHubs(l_cid, Util::emptyString);
		hubs.insert(hubs.end(), sl.begin(), sl.end());
#if 0
		Util::intersect(hubs, ClientManager::getHubs(l_cid));
#endif
	}
}

tstring SearchFrame::getDownloadDirectory(WORD wID)
{
	TargetsMap::const_iterator i = dlTargets.find(wID);
	tstring dir; // [!] IRainman replace Text::toT(SETTING(DOWNLOAD_DIRECTORY)) to Util::emptyStringT, its needs for support download to specify extension dir.
	if (wID == IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS) return Text::toT(SETTING(DOWNLOAD_DIRECTORY));
	if (i == dlTargets.end()) return dir;
	const auto& l_trarget = i->second; // [!] PVS V807 Decreased performance. Consider creating a reference to avoid using the 'i->second' expression repeatedly. searchfrm.cpp 1201
	if (l_trarget.Type == TARGET_STRUCT::PATH_BROWSE)
	{
		if (WinUtil::browseDirectory(dir, m_hWnd))
		{
			LastDir::add(dir);
			return dir;
		}
		else
		{
			return Util::emptyStringT;
		}
	}
	// [+] brain-ripper
	// I'd like to have such feature -
	// save chosen SRC path in menu.
	else if (l_trarget.Type == TARGET_STRUCT::PATH_SRC && !l_trarget.strPath.empty())
	{
		LastDir::add(i->second.strPath);
	}
	if (!l_trarget.strPath.empty()) dir = l_trarget.strPath;
	return dir;
}

LRESULT SearchFrame::onDownloadWithPrio(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	QueueItem::Priority p;
	
	switch (wID)
	{
		case IDC_PRIORITY_PAUSED:
			p = QueueItem::PAUSED;
			break;
		case IDC_PRIORITY_LOWEST:
			p = QueueItem::LOWEST;
			break;
		case IDC_PRIORITY_LOW:
			p = QueueItem::LOW;
			break;
		case IDC_PRIORITY_NORMAL:
			p = QueueItem::NORMAL;
			break;
		case IDC_PRIORITY_HIGH:
			p = QueueItem::HIGH;
			break;
		case IDC_PRIORITY_HIGHEST:
			p = QueueItem::HIGHEST;
			break;
		default:
			p = QueueItem::DEFAULT;
			break;
	}
	
	tstring dir = getDownloadDirectory(wID);
	if (!dir.empty())
		ctrlResults.forEachSelectedT(SearchInfo::Download(m_target, this, p));
	else
	{
		int i = -1;
		while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
		{
			const SearchInfo* si = ctrlResults.getItemData(i);
			dir = Text::toT(FavoriteManager::getDownloadDirectory(Util::getFileExt(si->m_sr.getFileName())));
			(SearchInfo::Download(dir, this, p))(si); //-V607
		}
	}
	return 0;
}

LRESULT SearchFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (ctrlResults.GetSelectedCount() == 1)
	{
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		const SearchInfo* si = ctrlResults.getItemData(i);
		const SearchResult& sr = si->m_sr;
		
		if (sr.getType() == SearchResult::TYPE_FILE)
		{
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + si->getText(COLUMN_FILENAME);
			if (WinUtil::browseFile(target, m_hWnd, true, Util::emptyStringT, NULL, Util::getFileExtWithoutDot(target).c_str()))
			{
				LastDir::add(Util::getFilePath(target));
				ctrlResults.forEachSelectedT(SearchInfo::DownloadTarget(target));
			}
		}
		else
		{
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
			if (WinUtil::browseDirectory(target, m_hWnd))
			{
				LastDir::add(target);
				ctrlResults.forEachSelectedT(SearchInfo::Download(target, this, QueueItem::DEFAULT));
			}
		}
	}
	else
	{
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if (WinUtil::browseDirectory(target, m_hWnd))
		{
			LastDir::add(target);
			ctrlResults.forEachSelectedT(SearchInfo::Download(target, this, QueueItem::DEFAULT));
		}
	}
	return 0;
}

LRESULT SearchFrame::onDownload(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const tstring dir = getDownloadDirectory(wID);
	if (!dir.empty())
		ctrlResults.forEachSelectedT(SearchInfo::Download(dir, this, QueueItem::DEFAULT));
	else
	{
		// [+] InfinitySky.
		int i = -1;
		while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
		{
			const SearchInfo* si = ctrlResults.getItemData(i);
			if (si->m_is_torrent == false)
			{
				const string t = FavoriteManager::getDownloadDirectory(Util::getFileExt(si->m_sr.getFileName()));
				(SearchInfo::Download(Text::toT(t), this, QueueItem::DEFAULT))(si);
			}
			else
			{
#ifdef FLYLINKDC_USE_TORRENT
				DownloadManager::getInstance()->add_torrent_file(_T(""), Text::toT(si->m_sr.getTorrentMagnet()));
#endif
			}
		}
	}
	return 0;
}

LRESULT SearchFrame::onDownloadWhole(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const tstring dir = getDownloadDirectory(wID);
	if (!dir.empty())
		ctrlResults.forEachSelectedT(SearchInfo::DownloadWhole(dir));
	return 0;
}

LRESULT SearchFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const tstring dir = getDownloadDirectory(wID);
	if (!dir.empty())
		ctrlResults.forEachSelectedT(SearchInfo::DownloadTarget(dir));
	return 0;
}
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
bool SearchFrame::showFlyServerProperty(const SearchInfo* p_item_info)
{
	CFlyServerDialogNavigator l_dlg;
	static const int l_mediainfo_array[] =
	{
		COLUMN_FLY_SERVER_RATING,
		COLUMN_BITRATE,
		COLUMN_MEDIA_XY,
		COLUMN_MEDIA_VIDEO,
		COLUMN_MEDIA_AUDIO,
		COLUMN_DURATION
	};
	for (int i = 0; i < _countof(l_mediainfo_array); ++i)
	{
		const auto l_MIItem = p_item_info->getText(l_mediainfo_array[i]);
		if (!l_MIItem.empty())
			l_dlg.m_MediaInfo.push_back(make_pair(CTSTRING_I(columnNames[l_mediainfo_array[i]]), p_item_info->getText(l_mediainfo_array[i])));
	}
	static const int l_fileinfo_array[] =
	{
		COLUMN_FILENAME,
		COLUMN_LOCAL_PATH,
		COLUMN_HITS,
		COLUMN_NICK,
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
		COLUMN_ANTIVIRUS,
#endif
		COLUMN_P2P_GUARD,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_HUB,
		COLUMN_EXACT_SIZE,
		COLUMN_LOCATION,
		COLUMN_IP,
		COLUMN_TTH
	};
	for (int i = 0; i < _countof(l_fileinfo_array); ++i)
	{
		l_dlg.m_FileInfo.push_back(make_pair(CTSTRING_I(columnNames[l_fileinfo_array[i]]), p_item_info->getText(l_fileinfo_array[i])));
	}
	const string l_inform = CFlyServerInfo::getMediaInfoAsText(p_item_info->m_sr.getTTH(), p_item_info->m_sr.getSize());
	l_dlg.m_MIInform.push_back(make_pair(_T("General"), Text::toT(l_inform)));
	const auto l_result = l_dlg.DoModal(m_hWnd);
	return l_result == IDOK;
	
}
#endif // FLYLINKDC_USE_MEDIAINFO_SERVER
LRESULT SearchFrame::onDoubleClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	const LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)pnmh;
	
	if (item->iItem != -1)
	{
		CRect rect;
		ctrlResults.GetItemRect(item->iItem, rect, LVIR_ICON);
		
		// if double click on state icon, ignore...
		if (item->ptAction.x < rect.left)
			return 0;
			
		// [+] InfinitySky.
		int i = -1;
		while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
		{
			const SearchInfo* si = ctrlResults.getItemData(i);
			if (si)
			{
				if (si->m_is_torrent == false)
				{
					const string t = FavoriteManager::getDownloadDirectory(Util::getFileExt(si->m_sr.getFileName()));
					(SearchInfo::Download(Text::toT(t), this, QueueItem::DEFAULT))(si);
				}
				else
				{
#ifdef FLYLINKDC_USE_TORRENT
					DownloadManager::getInstance()->add_torrent_file(_T(""), Text::toT(si->m_sr.getTorrentMagnet()));
#endif
				}
			}
		}
		//ctrlResults.forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY)), this, QueueItem::DEFAULT));
	}
	return 0;
}

LRESULT SearchFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & bHandled)
{
	closing = true;
	CWaitCursor waitCursor;
	if (!closed)
	{
		closed = true;
		safe_destroy_timer();
		SettingsManager::getInstance()->removeListener(this);
		if (!CompatibilityManager::isWine())
		{
			ClientManager::getInstance()->removeListener(this);
		}
		SearchManager::getInstance()->removeListener(this);
		g_search_frames.erase(m_hWnd);
		ctrlResults.DeleteAndClearAllItems();
		clearPausedResults();
		ctrlHubs.DeleteAndCleanAllItems();
		clear_tree_filter_contaners();
		ctrlResults.saveHeaderOrder(SettingsManager::SEARCH_FRAME_ORDER, SettingsManager::SEARCH_FRAME_WIDTHS, SettingsManager::SEARCH_FRAME_VISIBLE);
		SET_SETTING(SEARCH_FRAME_SORT, ctrlResults.getSortForSettings());
		PostMessage(WM_CLOSE);
		return 0;
	}
	else
	{
		bHandled = FALSE;
		return 0;
	}
}

void SearchFrame::UpdateLayout(BOOL bResizeBars)
{
	if (isClosedOrShutdown())
		return;
	m_tooltip.Activate(FALSE);
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	int const width = 222, spacing = 50, labelH = 16, comboH = 140, lMargin = 4, rMargin = 4, miniwidth = 30; // [+] InfinitySky. �������� �������� "miniwidth".
	
	if (ctrlStatus.IsWindow())
	{
		CRect sr;
		int w[5];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 420 ? 376 : ((sr.Width() > 116) ? sr.Width() - 100 : 16);
		
		w[0] = 42;
		w[1] = sr.right - tmp;
		w[2] = w[1] + (tmp - 16) / 3;
		w[3] = w[2] + (tmp - 16) / 3;
		w[4] = w[3] + (tmp - 16) / 3;
		
		ctrlStatus.SetParts(5, w);
		
		// Layout showUI button in statusbar part #0
		ctrlStatus.GetRect(0, sr);
		sr.left += 4;
		sr.right += 4;
		ctrlShowUI.MoveWindow(sr);
	}
	if (m_showUI)
	{
		CRect rc = rect;
#ifdef FLYLINKDC_USE_TREE_SEARCH
		const int l_width_tree = m_is_use_tree ? 200 : 0;
#else
		const int l_width_tree = 0;
#endif
		rc.left += width + l_width_tree;
		rc.bottom -= 26;
		CRect rc_result = rc;
		// TODO rc_result.bottom -= 100;
		ctrlResults.MoveWindow(rc_result);
		
#ifdef FLYLINKDC_USE_TREE_SEARCH
		if (m_is_use_tree)
		{
			CRect rc_tree = rc;
			rc_tree.left -= l_width_tree;
			rc_tree.right = rc_tree.left + l_width_tree - 5;
			m_ctrlSearchFilterTree.MoveWindow(rc_tree);
		}
#endif
		// "Search for".
		rc.left = lMargin; // ����� �������.
		rc.right = width - rMargin; // ������ �������.
		rc.top += 25; // ������� �������.
		rc.bottom = rc.top + comboH; // ������ �������.
		ctrlSearchBox.MoveWindow(rc);
		
		searchLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, 60, labelH - 1);
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
		CRect l_fly_rc = rc;
		l_fly_rc.top  = 5;
		l_fly_rc.left = 70;
		l_fly_rc.right = 90;
		l_fly_rc.bottom = 24;
		m_ctrlFlyServer.MoveWindow(l_fly_rc);
		
		l_fly_rc.top  = 7;
		l_fly_rc.left = 90;
		l_fly_rc.right = 216;
		l_fly_rc.bottom = 22;
		m_FlyServerGradientLabel.MoveWindow(l_fly_rc);
#endif
		// "Clear search history".
		rc.left = lMargin; // ����� �������.
		rc.right = rc.left + miniwidth; // ������ �������. [~] InfinitySky. " + miniwidth".
		rc.top += 25; // ������� �������.
		rc.bottom = rc.top + 24; // ������ �������. [~] InfinitySky.
		ctrlPurge.MoveWindow(rc);
		
		
		CRect l_tmp_rc = rc;
		
		// "Pause".
		rc.left += miniwidth + 4; // ����� �������. [~] InfinitySky. " + miniwidth".
		rc.right = rc.left + 88; // ������ �������. [~] InfinitySky.
		ctrlPauseSearch.MoveWindow(rc); // [<-] InfinitySky.
		
		// "Search".
		rc.left += 88 + 4; // ����� �������. [~] InfinitySky.
		rc.right = rc.left + 88; // ������ �������. [~] InfinitySky.
		ctrlDoSearch.MoveWindow(rc); // [<-] InfinitySky.
		
		// Search firewall
//		rc.left   -= 50;
//		rc.top    += 24;
//		rc.bottom += 17;


		rc = l_tmp_rc;
		
		// "Size".
		// ���������� ������ �������.
		int w2 = width - lMargin - rMargin;
		rc.top += spacing - 7; // ������� �������. [~] InfinitySky.
		rc.bottom = rc.top + comboH; // ������ �������.
		rc.right = w2 / 2; // ������ �������. [~] InfinitySky.
		ctrlMode.MoveWindow(rc);
		
		// ������� "������": (�����, �������, ������, ������ �������).
		sizeLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, 40 /* width - rMargin */, labelH - 1);
		
		// ���� ��� �����.
		rc.left = rc.right + lMargin; // ����� �������.
		rc.right += w2 / 4; // ������ �������. [~] InfinitySky.
		rc.bottom = rc.top + 22; // ������ �������. [~] InfinitySky.
		ctrlSize.MoveWindow(rc);
		
		// ���������� ������ �������� ���������.
		rc.left = rc.right + lMargin; // ����� �������.
		rc.right = width - rMargin; // ������ �������.
		rc.bottom = rc.top + comboH; // ������ �������.
		ctrlSizeMode.MoveWindow(rc);
		
		// "File type".
		rc.left = lMargin; // ����� �������.
		rc.right = width - rMargin; // ������ �������.
		rc.top += spacing - 9; // ������� �������. [~] InfinitySky.
		rc.bottom = rc.top + comboH + 21;
		ctrlFiletype.MoveWindow(rc);
		//rc.bottom -= comboH;
		
		// ������� "��� �����": (�����, �������, ������, ������ �������).
		typeLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH - 1);
		
#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH
		// "Search filter"
		const LONG gridFilterSMin     = 70;
		const LONG gridFilterBAnchor  = 530;
		
		LONG gridFilterH = rect.bottom - gridFilterBAnchor;
		
		rc.top += spacing;
		srLabel.MoveWindow(lMargin * 2, rc.top - labelH, width - rMargin, labelH - 1);
		
		rc.left     = -1;
		rc.right    = width + 1;
		rc.bottom   = rc.top + max(gridFilterSMin, gridFilterSMin + gridFilterH);
		ctrlGridFilters.MoveWindow(rc);
		
		rc.top += 10 + max(gridFilterSMin, gridFilterSMin + gridFilterH);
#endif
		// "Search options".
		rc.left = lMargin + 4;
		rc.right = width - rMargin;
		rc.top += spacing - 10;
		rc.bottom = rc.top + 17; // ������ �������. [~] InfinitySky.
		
		optionLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH - 1);
		ctrlSlots.MoveWindow(rc);
		
		rc.top += 17;       //21
		rc.bottom += 17;
		ctrlCollapsed.MoveWindow(rc);
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
		rc.top += 17;
		rc.bottom += 17;
		m_ctrlStoreIP.MoveWindow(rc);
#endif
		rc.top += 17;
		rc.bottom += 17;
		ctrlStoreSettings.MoveWindow(rc);
		
		rc.top += 17;
		rc.bottom += 17;
		ctrlUseGroupTreeSettings.MoveWindow(rc);
		
		rc.top += 17;
		rc.bottom += 17;
		ctrlUseTorrentSearch.MoveWindow(rc);
		
		rc.top += 17;
		rc.bottom += 17;
		ctrlUseTorrentRSS.MoveWindow(rc);
		
		// "Hubs".
		rc.left = lMargin;
		rc.right = width - rMargin;
		rc.top += spacing - 16;
		rc.bottom = rect.bottom - 5;
		if (rc.bottom < rc.top + (labelH * 3) / 2)
			rc.bottom = rc.top + (labelH * 3) / 2;
		ctrlHubs.MoveWindow(rc);
		
		if (!CompatibilityManager::isWine())
		{
			hubsLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH - 1);
		}
	}
	else   // if(m_showUI)
	{
		CRect rc = rect;
		rc.bottom -= 26;
		ctrlResults.MoveWindow(rc);
		
		rc.SetRect(0, 0, 0, 0);
		ctrlSearchBox.MoveWindow(rc);
		ctrlMode.MoveWindow(rc);
		ctrlPurge.MoveWindow(rc);
		ctrlSize.MoveWindow(rc);
		ctrlSizeMode.MoveWindow(rc);
		ctrlFiletype.MoveWindow(rc);
		ctrlPauseSearch.MoveWindow(rc);
		
		ctrlCollapsed.MoveWindow(rc);
		ctrlSlots.MoveWindow(rc);
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
		m_ctrlStoreIP.MoveWindow(rc);
#endif
		ctrlStoreSettings.MoveWindow(rc);
		ctrlUseGroupTreeSettings.MoveWindow(rc);
		ctrlUseTorrentSearch.MoveWindow(rc);
		ctrlUseTorrentRSS.MoveWindow(rc);
		
		ctrlHubs.MoveWindow(rc);
		//  srLabel.MoveWindow(rc);
		//  ctrlFilterSel.MoveWindow(rc);
		//  ctrlFilter.MoveWindow(rc);
		ctrlDoSearch.MoveWindow(rc);
		typeLabel.MoveWindow(rc);
		hubsLabel.MoveWindow(rc);
		sizeLabel.MoveWindow(rc);
		searchLabel.MoveWindow(rc);
		optionLabel.MoveWindow(rc);
	}
	
	CRect rc = rect;
	rc.bottom -= 26;
	// ����� "������ � ���������"
	rc.left += lMargin * 2;
	if (m_showUI)
		rc.left += width;
	rc.top = rc.bottom + 2;
	rc.bottom = rc.top + 22;
	rc.right = rc.left + WinUtil::getTextWidth(CTSTRING(SEARCH_IN_RESULTS), m_hWnd) - 20;
	srLabel.MoveWindow(rc.left, rc.top + 4, rc.right - rc.left, rc.bottom - rc.top);
	// ���� �����
	rc.left = rc.right + lMargin;
	rc.right = rc.left + 150;
	ctrlFilter.MoveWindow(rc);
	// ���������� ������.
	rc.left = rc.right + lMargin;
	rc.right = rc.left + 120;
	ctrlFilterSel.MoveWindow(rc);
	
	CRect rcIcon = rc;
	
//	rcIcon.left  = rcIcon.right + 3;
//	rcIcon.right = rcIcon.left + 90;
//	ctrlDoUDPTestPort.MoveWindow(rcIcon);

	rcIcon.left = rcIcon.right + 5;
	rcIcon.top  += 3;
	rcIcon.bottom -= 3;
	rcIcon.right = rcIcon.left + 16;
	ctrlUDPMode.MoveWindow(rcIcon);
	
	rcIcon.left += 19;
	rcIcon.right = rcIcon.left + 200;
	rcIcon.top++;
	ctrlUDPTestResult.MoveWindow(rcIcon);
	
	const POINT pt = {10, 10};
	const HWND hWnd = ctrlSearchBox.ChildWindowFromPoint(pt);
	if (hWnd != NULL && !ctrlSearch.IsWindow() && hWnd != ctrlSearchBox.m_hWnd)
	{
		ctrlSearch.Attach(hWnd);
		searchContainer.SubclassWindow(ctrlSearch.m_hWnd);
	}
	if (!BOOLSETTING(POPUPS_DISABLED))
		m_tooltip.Activate(TRUE);
}

void SearchFrame::runUserCommand(UserCommand & uc)
{
	if (!WinUtil::getUCParams(m_hWnd, uc, m_ucLineParams))
		return;
		
	StringMap ucParams = m_ucLineParams;
	
	std::set<CID> l_users;
	
	int sel = -1;
	while ((sel = ctrlResults.GetNextItem(sel, LVNI_SELECTED)) != -1)
	{
		const SearchResult& sr = ctrlResults.getItemData(sel)->m_sr;
		
		if (!sr.getUser())
			continue;
		if (!sr.getUser()->isOnline())
			continue;
			
		if (uc.getType() == UserCommand::TYPE_RAW_ONCE)
		{
			if (l_users.find(sr.getUser()->getCID()) != l_users.end())
				continue;
			l_users.insert(sr.getUser()->getCID());
		}
		
		ucParams["fileFN"] = sr.getFile();
		ucParams["fileSI"] = Util::toString(sr.getSize());
		ucParams["fileSIshort"] = Util::formatBytes(sr.getSize());
		if (sr.getType() == SearchResult::TYPE_FILE)
		{
			ucParams["fileTR"] = sr.getTTH().toBase32();
		}
		
		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		ucParams["filesize"] = ucParams["fileSI"];
		ucParams["filesizeshort"] = ucParams["fileSIshort"];
		ucParams["tth"] = ucParams["fileTR"];
		
		StringMap tmp = ucParams;
		ClientManager::userCommand(sr.getHintedUser(), uc, tmp, true);
	}
}

LRESULT SearchFrame::onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	const HWND hWnd = (HWND)lParam;
	const HDC hDC = (HDC)wParam;
	if (hWnd == searchLabel.m_hWnd || hWnd == sizeLabel.m_hWnd || hWnd == optionLabel.m_hWnd || hWnd == typeLabel.m_hWnd
	        || hWnd == hubsLabel.m_hWnd || hWnd == ctrlSlots.m_hWnd ||
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
	        hWnd == m_ctrlStoreIP.m_hWnd ||
#endif
	        hWnd == ctrlStoreSettings.m_hWnd ||
	        hWnd == ctrlUseGroupTreeSettings.m_hWnd ||
	        hWnd == ctrlUseTorrentSearch.m_hWnd ||
	        hWnd == ctrlUseTorrentRSS.m_hWnd ||
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
	        hWnd == m_ctrlFlyServer.m_hWnd ||
#endif
	        hWnd == ctrlCollapsed.m_hWnd || hWnd == srLabel.m_hWnd
	        || hWnd == ctrlUDPTestResult.m_hWnd || hWnd == ctrlUDPMode.m_hWnd
#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH
	        || hWnd == srLabelExcl.m_hWnd
#endif
	   )
	{
		::SetBkColor(hDC, ::GetSysColor(COLOR_3DFACE));
		::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT));
		return (LRESULT)::GetSysColorBrush(COLOR_3DFACE);
	}
	else
	{
		return Colors::setColor(hDC);
	}
}

LRESULT SearchFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled)
{
	switch (wParam)
	{
		case VK_TAB:
			if (uMsg == WM_KEYDOWN)
			{
				onTab(WinUtil::isShift());
			}
			break;
		case VK_RETURN:
			if (WinUtil::isShift() || WinUtil::isCtrlOrAlt())
			{
				bHandled = FALSE;
			}
			else
			{
				if (uMsg == WM_KEYDOWN)
				{
					onEnter();
				}
			}
			break;
		default:
			bHandled = FALSE;
	}
	return 0;
}

void SearchFrame::onTab(bool shift)
{
	static const HWND wnds[] =
	{
		ctrlSearch.m_hWnd, ctrlPurge.m_hWnd, ctrlMode.m_hWnd, ctrlSize.m_hWnd, ctrlSizeMode.m_hWnd,
		ctrlFiletype.m_hWnd, ctrlSlots.m_hWnd, ctrlCollapsed.m_hWnd,
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
		m_ctrlStoreIP.m_hWnd,
#endif
#ifdef FLYLINKDC_USE_MEDIAINFO_SERVER
		m_ctrlFlyServer.m_hWnd,
#endif
		ctrlStoreSettings.m_hWnd, ctrlDoSearch.m_hWnd, ctrlSearch.m_hWnd,
		ctrlResults.m_hWnd,
		ctrlUseGroupTreeSettings.m_hWnd,
		ctrlUseTorrentSearch.m_hWnd,
		ctrlUseTorrentRSS.m_hWnd
	};
	
	HWND focus = GetFocus();
	if (focus == ctrlSearchBox.m_hWnd)
		focus = ctrlSearch.m_hWnd;
		
	const int size = _countof(wnds);
	int i;
	for (i = 0; i < size; i++)
	{
		if (wnds[i] == focus)
			break;
	}
	
	::SetFocus(wnds[(i + (shift ? -1 : 1)) % size]);
}

LRESULT SearchFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	const POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click
	
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
//	tabMenu.RemoveFirstItem();
//	for (int i = tabMenu.GetMenuItemCount(); i; i--)
//		tabMenu.DeleteMenu(0, MF_BYPOSITION);
//	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
//	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
	cleanUcMenu(tabMenu);
	return TRUE;
}

LRESULT SearchFrame::onTabGetOptions(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	FlatTabOptions* opt = reinterpret_cast<FlatTabOptions*>(lParam);
	opt->icons[0] = opt->icons[1] = g_search_icon;
	opt->isHub = false;
	return TRUE;
}

LRESULT SearchFrame::onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// !SMT!-UI
	int i = -1;
	while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
	{
		const SearchResult sr = ctrlResults.getItemData(i)->m_sr;
		if (sr.getType() == SearchResult::TYPE_FILE)
		{
			WinUtil::searchHash(sr.getTTH());
		}
	}
	return 0;
}

#ifdef FLYLINKDC_USE_TREE_SEARCH

LRESULT SearchFrame::onSelChangedTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	if (closed) return 0;
	NMTREEVIEW* p = (NMTREEVIEW*)pnmh;
	m_CurrentTreeItem = p->itemNew.hItem;
	CLockRedraw<> l_lock_draw(ctrlResults);
	ctrlResults.DeleteAllItems();
	CFlyLock(m_filter_map_cs);
	const auto& l_filtered_item = m_filter_map[m_CurrentTreeItem];
	if (l_filtered_item.empty() || m_CurrentTreeItem == m_RootTreeItem)
	{
		updateSearchList(nullptr);
	}
#ifdef _DEBUG
	boost::unordered_set<SearchInfo*> l_dup_filter;
#endif
	const auto& l_virus = m_filter_map[m_RootVirusTreeItem];
	
	for (auto i = l_filtered_item.cbegin(); i != l_filtered_item.cend(); ++i)
	{
		SearchInfo* l_si = i->first;
		l_si->collapsed = true;
#ifdef _DEBUG
		const auto l_dup_res = l_dup_filter.insert(l_si);
		dcassert(l_dup_res.second == true);
#endif
		//ctrlResults.insertGroupedItem(l_si, m_expandSR, false, true);
		if (m_CurrentTreeItem != m_RootVirusTreeItem)
		{
			bool l_is_virus = false;
			for (auto j : l_virus)
			{
				if (j.first == l_si)
				{
					l_is_virus = true;
					break;
				}
			}
			if (l_is_virus == true)
				continue;
		}
		set_tree_item_status(l_si);
	}
	return 0;
}

bool SearchFrame::is_filter_item(const SearchInfo* si)
{
	if (m_CurrentTreeItem == m_RootTreeItem || m_CurrentTreeItem == nullptr)
		return true;
	else
	{
		bool l_is_filter = false;
		if (si->m_is_torrent == false && si->m_sr.getType() == SearchResult::TYPE_FILE)
		{
			const auto l_file_ext = Text::toLower(Util::getFileExtWithoutDot(si->m_sr.getFileName()));
			CFlyLock(m_filter_map_cs);
			const auto& l_filtered_item = m_filter_map[m_CurrentTreeItem];
			for (auto j = l_filtered_item.cbegin(); j != l_filtered_item.cend(); ++j)
			{
				if (j->second == l_file_ext)
				{
					l_is_filter = true;
					break;
				}
			}
		}
		else if (si->m_is_torrent && si->m_sr.getType() == SearchResult::TYPE_TORRENT_MAGNET)
		{
			if (m_CurrentTreeItem == m_tree_type[FILE_TYPE_TORRENT_MAGNET])
				l_is_filter = true;
			else
				l_is_filter = false;
			/*
			CFlyLock(m_filter_map_cs);
			const auto& l_filtered_item = m_filter_map[m_CurrentTreeItem];
			for (auto j = l_filtered_item.cbegin(); j != l_filtered_item.cend(); ++j)
			{
			    //if (j->second == l_file_ext)
			    {
			        l_is_filter = true;
			        break;
			    }
			}
			*/
		}
		return l_is_filter;
	}
}
#endif // FLYLINKDC_USE_TREE_SEARCH

bool SearchFrame::isSkipSearchResult(SearchInfo*& si)
{
	//dcassert(!closed);
	if (closed || m_is_before_search)
	{
		check_delete(si);
		return true;
	}
	else
	{
		return false;
	}
}

void SearchFrame::addSearchResult(SearchInfo* si)
{
	if (isSkipSearchResult(si))
		return;
	const SearchResult& sr = si->m_sr;
	SearchInfoList::ParentPair* pp = nullptr;
	if (!si->m_is_torrent)
	{
		const auto l_user      = sr.getUser();
		if (!sr.getIP().is_unspecified())
		{
			l_user->setIP(sr.getIP(), true);
		}
		// Check previous search results for dupes
		if (!si->getText(COLUMN_TTH).empty())
		{
			pp = ctrlResults.findParentPair(sr.getTTH());
			if (pp)
			{
				if (pp->parent && // fix https://drdump.com/DumpGroup.aspx?DumpGroupID=1052556
				        l_user->getCID() == pp->parent->getUser()->getCID() && sr.getFile() == pp->parent->m_sr.getFile())
				{
					check_delete(si);
					return;
				}
				for (auto k = pp->children.cbegin(); k != pp->children.cend(); ++k)
				{
					if (isSkipSearchResult(si))
						return;
					if (l_user->getCID() == (*k)->getUser()->getCID())
					{
						if (sr.getFile() == (*k)->m_sr.getFile())
						{
							check_delete(si);
							return;
						}
					}
				}
			}
		}
		else
		{
			for (auto s = ctrlResults.getParents().cbegin(); s != ctrlResults.getParents().cend(); ++s)
			{
				if (isSkipSearchResult(si))
					return;
				const SearchInfo* si2 = s->second.parent;
				const auto& sr2 = si2->m_sr;
				if (l_user->getCID() == sr2.getUser()->getCID())
				{
					if (sr.getFile() == sr2.getFile())
					{
						check_delete(si);
						return;
					}
				}
			}
		}
	}
	if (m_running || si->m_is_torrent)
	{
		resultsCount++;
#ifdef FLYLINKDC_USE_TREE_SEARCH
		if (si->m_is_top_torrent)
		{
			if (!m_RootTorrentRSSTreeItem)
			{
				m_RootTorrentRSSTreeItem = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
				                                                             _T("Torrent RSS"),
				                                                             0, // nImage
				                                                             0, // nSelectedImage
				                                                             0, // nState
				                                                             0, // nStateMask
				                                                             e_Root, // lParam
				                                                             0, // aParent,
				                                                             0  // hInsertAfter
				                                                            );
			}
			if (sr.m_group_name.empty())
			{
				m_24HTopTorrentTreeItem = add_category("...in 24 hours", "Hits", si, sr, SearchResult::TYPE_TORRENT_MAGNET, m_RootTorrentRSSTreeItem, true, true);
			}
			else
			{
				add_category(sr.m_group_name, "Categories", si, sr, SearchResult::TYPE_TORRENT_MAGNET, m_RootTorrentRSSTreeItem, true, true);
			}
			const auto l_marker = make_pair(si, ".torrent-magnet-top");
			{
				CFlyLock(m_filter_map_cs);
				for (auto const &c : m_category_map)
				{
					m_filter_map[c.second].push_back(l_marker);
				}
				m_filter_map[m_RootTorrentRSSTreeItem].push_back(l_marker);
			}
		}
		else
		{
			if (!m_RootTreeItem)
			{
				m_RootTreeItem = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
				                                                   _T("Search"),
				                                                   0, // nImage
				                                                   0, // nSelectedImage
				                                                   0, // nState
				                                                   0, // nStateMask
				                                                   e_Root, // lParam
				                                                   0, // aParent,
				                                                   0  // hInsertAfter
				                                                  );
			}
			if (sr.getType() == SearchResult::TYPE_FILE)
			{
				const auto l_file = sr.getFileName();
				const auto l_file_type = ShareManager::getFType(l_file, true);
				auto& l_type_node = m_tree_type[l_file_type];
				if (l_type_node == nullptr)
				{
					l_type_node = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
					                                                l_file_type == FILE_TYPE_ANY ? CTSTRING(OTHER) : Text::toT(SearchManager::getTypeStr(l_file_type)).c_str(),
					                                                l_file_type, // nImage
					                                                l_file_type, // nSelectedImage
					                                                0, // nState
					                                                0, // nStateMask
					                                                l_file_type, // lParam
					                                                m_RootTreeItem, // aParent,
					                                                0  // hInsertAfter
					                                               );
					if (m_is_expand_tree == false)
					{
						m_ctrlSearchFilterTree.Expand(m_RootTreeItem);
						m_is_expand_tree = true;
					}
				}
				HTREEITEM l_item = nullptr;
				const auto l_file_ext = Text::toLower(Util::getFileExtWithoutDot(l_file));
				// Virus?
#if 0
				{
					if (l_file_ext.compare(0, 3, "exe", 3) == 0)
					{
						if ((si->m_sr.m_size <= CFlyServerConfig::g_max_size_search_v_detect) ||
						        CFlyServerConfig::isVirusEnd(Text::toLower(l_file)))
						{
							if (!m_RootVirusTreeItem)
							{
								m_skull_index = 15;
								m_RootVirusTreeItem = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
								                                                        _T("Virus ???"),
								                                                        m_skull_index, // nImage
								                                                        m_skull_index, // nSelectedImage
								                                                        0, // nState
								                                                        0, // nStateMask
								                                                        l_file_type, // lParam
								                                                        NULL, // aParent,
								                                                        0  // hInsertAfter
								                                                       );
							}
							l_item = m_RootVirusTreeItem;
							const auto l_marker = make_pair(si, l_file_ext);
							CFlyLock(m_filter_map_cs);
							m_filter_map[l_item].push_back(l_marker);
						}
						if (l_file_type == Search::TYPE_EXECUTABLE)
						{
							//const auto l_marker = make_pair(si, l_file_ext);
							//CFlyLock(m_filter_map_cs);
							//m_filter_map[l_type_node].push_back(l_marker);
						}
					}
				}
#endif
				if (l_item == nullptr)
				{
					const auto l_ext_item = m_tree_ext_map.find(l_file_ext);
					if (l_ext_item == m_tree_ext_map.end())
					{
						l_item = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
						                                           Text::toT(l_file_ext).c_str(),
						                                           0, // nImage
						                                           0, // nSelectedImage
						                                           0, // nState
						                                           0, // nStateMask
						                                           e_Ext, // lParam
						                                           l_type_node, // aParent,
						                                           0  // hInsertAfter
						                                          );
						m_tree_ext_map.insert(make_pair(l_file_ext, l_item));
					}
					else
					{
						l_item = l_ext_item->second;
					}
					const auto l_marker = make_pair(si, l_file_ext);
					CFlyLock(m_filter_map_cs);
					m_filter_map[l_item].push_back(l_marker);
					m_filter_map[l_type_node].push_back(l_marker);
				}
			}
			else if (sr.getType() == SearchResult::TYPE_TORRENT_MAGNET)
			{
				const auto l_file_type = FILE_TYPE_TORRENT_MAGNET;
				auto& l_torrent_node = m_tree_type[FILE_TYPE_TORRENT_MAGNET];
				if (l_torrent_node == nullptr)
				{
					l_torrent_node = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
					                                                   _T("Torrents"),
					                                                   l_file_type, // nImage
					                                                   l_file_type, // nSelectedImage
					                                                   0, // nState
					                                                   0, // nStateMask
					                                                   l_file_type, // lParam
					                                                   m_RootTreeItem, // aParent,
					                                                   0  // hInsertAfter
					                                                  );
					if (m_is_expand_tree == false)
					{
						m_ctrlSearchFilterTree.Expand(m_RootTreeItem);
						m_is_expand_tree = true;
					}
				}
				{
					const auto l_file_name = sr.getFileName();
					if (l_file_name.find(" PC ") != string::npos)
					{
						add_category("PC", "Soft", si, sr, l_file_type, l_torrent_node);
						add_category("DVD-ISO", "Soft", si, sr, l_file_type, l_torrent_node);
					}
					add_category(sr.m_tracker, "Tracker", si, sr, l_file_type, l_torrent_node, true);
					
					for (int j = 2020; j > 1900; --j)
					{
						add_category(Util::toString(j), "Year", si, sr, l_file_type, l_torrent_node);
					}
					{
						const char* l_doc_array[] = {
							"PDF",
							"RTF",
							"DJVU",
							"FB2"
						};
						for (const auto r : l_doc_array)
						{
							add_category(r, "Doc", si, sr, l_file_type, l_torrent_node);
						}
					}
					{
						const char* l_audio_rip_array[] = {
							"MP3",
							"FLAC"
						};
						for (const auto r : l_audio_rip_array)
						{
							add_category(r, "Audio", si, sr, l_file_type, l_torrent_node);
						}
					}
					
					{
						const char* l_type_rip_array[] = {
							"DVDRip",
							"DVD-Audio",
							"DVD-Video",
							"DVD-5",
							"DVD-9",
							"HDRip",
							"BDRip",
							"XviD",
							"BDRemux",
							"HDVideo",
							"HDTV",
							"CAMRip",
							"TS",
							"SATRip",
							"WEB-DLRip",
							"VHSRip",
							"iTunes",
							"Amedia",
							"���"
						};
						for (const auto r : l_type_rip_array)
						{
							add_category(r, "Video", si, sr, l_file_type, l_torrent_node);
						}
					}
					{
						const char* l_type_team_array[] = { "MediaClub",
						                                    "Files-x",
						                                    "GeneralFilm",
						                                    "MegaPeer",
						                                    "Neofilm",
						                                    "SeadLine Studio",
						                                    "Scarabey",
						                                    "den904",
						                                    "qqss44",
						                                    "HQ-ViDEO",
						                                    "SMALL-RiP",
						                                    "HQClub",
						                                    "HELLYWOOD",
						                                    "FreeHD",
						                                    "HDReactor",
						                                    "R.G."
						                                  };
						for (const auto t : l_type_team_array)
						{
							add_category(t, "Team", si, sr, l_file_type, l_torrent_node);
						}
					}
					{
						const char* l_type_resolution_array[] = {
							"1080p",
							"720p"
							"x264",
						};
						for (const auto r : l_type_resolution_array)
						{
							add_category(r, "Resolution", si, sr, l_file_type, l_torrent_node);
						}
					}
				}
				const auto l_marker = make_pair(si, ".torrent-magnet");
				{
					CFlyLock(m_filter_map_cs);
					for (auto const &c : m_category_map)
					{
						m_filter_map[c.second].push_back(l_marker);
					}
					m_filter_map[l_torrent_node].push_back(l_marker);
				}
			}
		}
#endif
		{
			CLockRedraw<> l_lock_draw(ctrlResults); //[+]IRainman optimize SearchFrame
			const SearchInfoList::ParentPair l_pp = { si, SearchInfoList::g_emptyVector };
			if (si->m_is_torrent)
			{
				if (is_filter_item(si))
				{
					ctrlResults.insertItem(si, I_IMAGECALLBACK);
				}
			}
			else
			{
				if (!si->getText(COLUMN_TTH).empty())
				{
#ifdef FLYLINKDC_USE_TREE_SEARCH
					const bool l_is_filter_ok = is_filter_item(si);
					if (l_is_filter_ok)
#endif
					{
						if (isSkipSearchResult(si))
							return;
						ctrlResults.insertGroupedItem(si, m_expandSR, false, true);
					}
					else
					{
						dcassert(!closed);
						if (pp == nullptr)
						{
							if (isSkipSearchResult(si))
								return;
							ctrlResults.getParents().insert(make_pair(sr.getTTH(), l_pp));
						}
						else
						{
							if (isSkipSearchResult(si))
								return;
							ctrlResults.insertChildNonVisual(si, pp, false, false, true);
						}
					}
				}
				else
				{
					if (isSkipSearchResult(si))
						return;
#ifdef FLYLINKDC_USE_TREE_SEARCH
					if (m_CurrentTreeItem == m_RootTreeItem || m_CurrentTreeItem == nullptr)
#endif
					{
						ctrlResults.insertItem(si, I_IMAGECALLBACK);
					}
					ctrlResults.getParents().insert(make_pair(sr.getTTH(), l_pp));
				}
			}
		}
		if (!filter.empty())
		{
			updateSearchList(si);
		}
		if (BOOLSETTING(BOLD_SEARCH))
		{
			setDirty();
		}
		m_need_resort = true;
	}
	else   // searching is paused, so store the result but don't show it in the GUI (show only information: visible/all results)
	{
#ifdef FLYLINK_DC_USE_PAUSED_SEARCH
		m_pausedResults.push_back(si);
		//ctrlStatus.SetText(3, (Util::toStringW(resultsCount + pausedResults.size()) + _T('/') + Util::toStringW(resultsCount) + _T(' ') + WSTRING(FILES)).c_str());//[-]IRainman optimize SearchFrame
#endif
	}
}

HTREEITEM SearchFrame::add_category(const std::string p_search, const std::string p_group, SearchInfo* p_si,
                                    const SearchResult& p_sr, int p_type_node, HTREEITEM p_parent_node,
                                    bool p_force_add /* = false */, bool p_expand /*= false */)
{
	HTREEITEM l_item = nullptr;
	const string l_year = p_search;
	const auto l_file_name = p_sr.getFileName();
	if (l_file_name.find(l_year) != string::npos || p_force_add == true)
	{
		const auto l_sub_item = m_tree_sub_torrent_map.find(l_year);
		if (l_sub_item == m_tree_sub_torrent_map.end())
		{
			auto& l_group_node = m_category_map[p_group];
			if (!l_group_node)
			{
				l_group_node = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
				                                                 Text::toT(p_group).c_str(),
				                                                 FILE_TYPE_TORRENT_MAGNET + 2, // nImage
				                                                 FILE_TYPE_TORRENT_MAGNET + 2, // nSelectedImage
				                                                 0, // nState
				                                                 0, // nStateMask
				                                                 p_type_node, // lParam
				                                                 p_parent_node, // aParent,
				                                                 0  // hInsertAfter
				                                                );
			}
			else
			{
				if (p_expand)
				{
					m_ctrlSearchFilterTree.Expand(l_group_node);
				}
			}
			l_item = m_ctrlSearchFilterTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
			                                           Text::toT(l_year).c_str(),
			                                           FILE_TYPE_TORRENT_MAGNET + 1, // nImage
			                                           FILE_TYPE_TORRENT_MAGNET + 1, // nSelectedImage
			                                           0, // nState
			                                           0, // nStateMask
			                                           e_Ext, // lParam
			                                           l_group_node, // aParent,
			                                           0  // hInsertAfter
			                                          );
			m_tree_sub_torrent_map.insert(std::make_pair(l_year, l_item));
			if (m_is_expand_sub_tree == false)
			{
				m_ctrlSearchFilterTree.Expand(p_parent_node);
				m_ctrlSearchFilterTree.Expand(l_group_node);
				m_is_expand_sub_tree = true;
			}
		}
		else
		{
			l_item = l_sub_item->second;
		}
		const auto l_marker = make_pair(p_si, ".torrent-magnet");
		{
			CFlyLock(m_filter_map_cs);
			m_filter_map[l_item].push_back(l_marker);
		}
	}
	return l_item;
}

LRESULT SearchFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	dcassert(!isClosedOrShutdown());
	if (isClosedOrShutdown())
		return 0;
	switch (wParam)
	{
		case ADD_RESULT:
			addSearchResult((SearchInfo*)(lParam));
			break;
		case PREPARE_RESULT_TOP_TORRENT:
		{
			dcassert(BOOLSETTING(USE_TORRENT_RSS));
			if (isClosedOrShutdown())
				return 0;
			SearchResult* l_sr = (SearchResult*)(lParam);
			auto l_ptr = new SearchInfo(*l_sr);
			delete l_sr;
			{
				CFlyFastLock(m_si_set_cs);
				m_si_set.insert(l_ptr);
			}
			l_ptr->m_is_torrent = true;
			l_ptr->m_is_top_torrent = true;
			addSearchResult(l_ptr);
		}
		break;
		case PREPARE_RESULT_TORRENT:
		{
			dcassert(BOOLSETTING(USE_TORRENT_SEARCH));
			
			if (isClosedOrShutdown())
				return 0;
			SearchResult* l_sr = (SearchResult*)(lParam);
			auto l_ptr = new SearchInfo(*l_sr);
			{
				CFlyFastLock(m_si_set_cs);
				m_si_set.insert(l_ptr);
			}
			delete l_sr;
			//l_ptr->m_torrent_page = z;
			l_ptr->m_is_torrent = true;
			addSearchResult(l_ptr);
		}
		break;
		case HUB_ADDED:
			onHubAdded((HubInfo*)(lParam));
			break;
		case HUB_CHANGED:
			onHubChanged((HubInfo*)(lParam));
			break;
		case HUB_REMOVED:
			onHubRemoved((HubInfo*)(lParam));
			break;
	}
	
	return 0;
}

int SearchFrame::makeTargetMenu(const SearchInfo* p_si)
{
	// first sub-menu
	while (targetMenu.GetMenuItemCount() > 0)
	{
		targetMenu.DeleteMenu(0, MF_BYPOSITION);
	}
	
	dlTargets[IDC_DOWNLOAD_FAVORITE_DIRS + 0] = TARGET_STRUCT(Util::emptyStringT, TARGET_STRUCT::PATH_DEFAULT); // for 'Download' without options
	
	targetMenu.InsertSeparatorFirst(TSTRING(DOWNLOAD_TO));
	
	int n = 1;
	FavoriteManager::LockInstanceDirs lockedInstance;
	const auto& spl = lockedInstance.getFavoriteDirsL();
	if (!spl.empty())
	{
		for (auto i = spl.cbegin(); i != spl.cend(); ++i)
		{
			const tstring tar = Text::toT(i->name); // !SMT!-S
			dlTargets[IDC_DOWNLOAD_FAVORITE_DIRS + n] = TARGET_STRUCT(Text::toT(i->dir), TARGET_STRUCT::PATH_FAVORITE);
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS + n, tar.c_str());
			n++;
		}
		targetMenu.AppendMenu(MF_SEPARATOR);
	}
	
	// !SMT!-S: Append special folder, like in source share
	if (p_si && p_si->m_is_torrent == false)
	{
		tstring srcpath = p_si->getText(COLUMN_PATH);
		if (srcpath.length() > 2)
		{
			size_t start = srcpath.substr(0, srcpath.length() - 1).find_last_of(_T('\\'));
			if (start == srcpath.npos) start = 0;
			else start++;
			srcpath = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + srcpath.substr(start);
			dlTargets[IDC_DOWNLOAD_FAVORITE_DIRS + n] = TARGET_STRUCT(srcpath, TARGET_STRUCT::PATH_SRC);
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS + n, srcpath.c_str());
			n++;
		}
	}
	
	targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CTSTRING(BROWSE));
	n++;
	
	//Append last favorite download dirs
	if (!LastDir::get().empty())
	{
		targetMenu.InsertSeparatorLast(TSTRING(PREVIOUS_FOLDERS));
		for (auto i = LastDir::get().cbegin(); i != LastDir::get().cend(); ++i)
		{
			const tstring& tar = *i; // !SMT!-S
			dlTargets[IDC_DOWNLOAD_FAVORITE_DIRS + n] = TARGET_STRUCT(tar, TARGET_STRUCT::PATH_LAST);
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS + n, Text::toLabel(tar).c_str());
			n++;
		}
	}
	return n;
}

LRESULT SearchFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if (reinterpret_cast<HWND>(wParam) == ctrlResults && ctrlResults.GetSelectedCount() > 0)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if (pt.x == -1 && pt.y == -1)
		{
			WinUtil::getContextMenuPos(ctrlResults, pt);
		}
		const auto l_sel_count = ctrlResults.GetSelectedCount();
		if (l_sel_count == 1)
		{
			OMenu resultsMenu;
			resultsMenu.CreatePopupMenu();
			resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS, CTSTRING(DOWNLOAD));
			//resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetMenu, CTSTRING(DOWNLOAD_TO));
			resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenuTorrent, CTSTRING(COPY));
			resultsMenu.AppendMenu(MF_SEPARATOR);
			const SearchInfo* l_si = nullptr;
			int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
			dcassert(i != -1);
			l_si = ctrlResults.getItemData(i);
			if (l_si->m_is_torrent)
			{
				makeTargetMenu(l_si);
				resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				return TRUE;
			}
		}
		if (l_sel_count)
		{
			clearUserMenu();
			// [+] brain-ripper
			// Make menu dynamic, since its content depends of which
			// user selected (for add/remove favorites item)
			OMenu resultsMenu;
			
			resultsMenu.CreatePopupMenu();
			
			resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS, CTSTRING(DOWNLOAD));
			resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetMenu, CTSTRING(DOWNLOAD_TO));
			resultsMenu.AppendMenu(MF_POPUP, (HMENU)priorityMenu, CTSTRING(DOWNLOAD_WITH_PRIORITY));
			resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS, CTSTRING(DOWNLOAD_WHOLE_DIR));
			resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetDirMenu, CTSTRING(DOWNLOAD_WHOLE_DIR_TO));
			resultsMenu.AppendMenu(MF_SEPARATOR);
#ifdef FLYLINKDC_USE_VIEW_AS_TEXT_OPTION
			resultsMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
#endif
			resultsMenu.AppendMenu(MF_SEPARATOR);
			resultsMenu.AppendMenu(MF_STRING, IDC_MARK_AS_DOWNLOADED, CTSTRING(MARK_AS_DOWNLOADED));
			resultsMenu.AppendMenu(MF_SEPARATOR);
			resultsMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
			
			resultsMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenu, CTSTRING(COPY));
			resultsMenu.AppendMenu(MF_SEPARATOR);
			appendAndActivateUserItems(resultsMenu, true);
			resultsMenu.AppendMenu(MF_SEPARATOR);
			resultsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
			resultsMenu.SetMenuDefaultItem(IDC_DOWNLOAD_FAVORITE_DIRS);
			
			dlTargets.clear(); // !SMT!-S
			
			const SearchInfo* l_si = nullptr;
			SearchResult sr;
			if (ctrlResults.GetSelectedCount() == 1)
			{
				int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
				dcassert(i != -1);
				l_si = ctrlResults.getItemData(i);
				sr = l_si->m_sr;
			}

			// Add target menu
			int n = makeTargetMenu(l_si);
			
			const SearchInfo::CheckTTH SIcheck = ctrlResults.forEachSelectedT(SearchInfo::CheckTTH());
			
			if (SIcheck.hasTTH)
			{
				targets.clear();
				QueueManager::getTargets(TTHValue(Text::fromT(SIcheck.tth)), targets);
				
				if (!targets.empty())
				{
					targetMenu.InsertSeparatorLast(TSTRING(ADD_AS_SOURCE));
					for (auto i = targets.cbegin(); i != targets.cend(); ++i)
					{
						const tstring tar = Text::toT(*i); // !SMT!-S
						dlTargets[IDC_DOWNLOAD_TARGET + n] = TARGET_STRUCT(tar, TARGET_STRUCT::PATH_DEFAULT);
						targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n,  Text::toLabel(tar).c_str());
						n++;
					}
				}
			}
			
			// second sub-menu
			while (targetDirMenu.GetMenuItemCount() > 0)
			{
				targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
			}
			
			dlTargets[IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + 0] = TARGET_STRUCT(Util::emptyStringT, TARGET_STRUCT::PATH_DEFAULT); // for 'Download whole dir' without options
			targetDirMenu.InsertSeparatorFirst(TSTRING(DOWNLOAD_WHOLE_DIR_TO));
			//Append favorite download dirs
			n = 1;
			{
				FavoriteManager::LockInstanceDirs lockedInstance;
				const auto& spl = lockedInstance.getFavoriteDirsL();
				if (!spl.empty())
				{
					for (auto i = spl.cbegin(); i != spl.cend(); ++i)
					{
						const tstring tar = Text::toT(i->name); // !SMT!-S
						dlTargets[IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n] = TARGET_STRUCT(Text::toT(i->dir), TARGET_STRUCT::PATH_DEFAULT);
						targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, tar.c_str());
						n++;
					}
					targetDirMenu.AppendMenu(MF_SEPARATOR);
				}
			}
			
			dlTargets[IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n] = TARGET_STRUCT(Util::emptyStringT, TARGET_STRUCT::PATH_BROWSE);
			targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, CTSTRING(BROWSE));
			n++;
			
			if (!LastDir::get().empty())
			{
				targetDirMenu.AppendMenu(MF_SEPARATOR);
				for (auto i = LastDir::get().cbegin(); i != LastDir::get().cend(); ++i)
				{
					const tstring tar = *i; // !SMT!-S
					dlTargets[IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n] = TARGET_STRUCT(tar, TARGET_STRUCT::PATH_LAST);
					targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, Text::toLabel(*i).c_str());
					n++;
				}
			}
			// [+] SCALOlaz: prepare for swap Item text
			const int l_ipos = WinUtil::GetMenuItemPosition(copyMenu, IDC_COPY_FILENAME);
			
			if (ctrlResults.GetSelectedCount() == 1 && sr.getType() == SearchResult::TYPE_FILE)
			{
				// [+] SCALOlaz: swap Item text
				if (l_ipos != -1)
					copyMenu.ModifyMenu(l_ipos, MF_BYPOSITION | MF_STRING, IDC_COPY_FILENAME, CTSTRING(FILENAME));
			}
			else if (ctrlResults.GetSelectedCount() == 1 && sr.getType() == SearchResult::TYPE_DIRECTORY)
			{
				// [+] SCALOlaz: swap Item text
				if (l_ipos != -1)
					copyMenu.ModifyMenu(l_ipos, MF_BYPOSITION | MF_STRING, IDC_COPY_FILENAME, CTSTRING(FOLDERNAME));
			}
			appendUcMenu(resultsMenu, UserCommand::CONTEXT_SEARCH, SIcheck.hubs);
			
			copyMenu.InsertSeparatorFirst(TSTRING(USERINFO));
			tstring caption;
			if (!sr.getFileName().empty())
			{
				caption = Text::toT(sr.getFileName());
				Text::limitStringLength(caption);
			} else caption = TSTRING(FILES);
			resultsMenu.InsertSeparatorFirst(caption);
			resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			resultsMenu.RemoveFirstItem();
			copyMenu.RemoveFirstItem();
			
			//cleanMenu(resultsMenu);
			return TRUE;
		}
	}
	bHandled = FALSE;
	return FALSE;
}

void SearchFrame::initHubs()
{
	if (!CompatibilityManager::isWine())
	{
		CLockRedraw<> l_lock_draw(ctrlHubs); // [+] IRainman opt.
		
		ctrlHubs.insertItem(new HubInfo(Util::emptyStringT, TSTRING(ONLY_WHERE_OP), false), 0);
		ctrlHubs.SetCheckState(0, false);
		ClientManager::getInstance()->addListener(this);
#ifdef IRAINMAN_NON_COPYABLE_CLIENTS_IN_CLIENT_MANAGER
		{
			ClientManager::LockInstanceClients l_CMinstanceClients;
			const auto& clients = l_CMinstanceClients->getClientsL();
			for (auto it = clients.cbegin(); it != clients.cend(); ++it)
			{
				const auto& client = it->second;
				if (client->isConnected())
					onHubAdded(new HubInfo(Text::toT(client->getHubUrl()), Text::toT(client->getHubName()), client->getMyIdentity().isOp()));
			}
		}
#else
		ClientManager::HubInfoArray l_hub_info;
		ClientManager::getConnectedHubInfo(l_hub_info);
		for (auto i = l_hub_info.cbegin(); i != l_hub_info.cend(); ++i)
		{
			onHubAdded(new HubInfo(Text::toT(i->m_hub_url), Text::toT(i->m_hub_name), i->m_is_op));
		}
#endif // IRAINMAN_NON_COPYABLE_CLIENTS_IN_CLIENT_MANAGER
		ctrlHubs.SetColumnWidth(0, LVSCW_AUTOSIZE);
	}
	else
	{
		ctrlHubs.ShowWindow(FALSE);
	}
}

void SearchFrame::onHubAdded(HubInfo* info)
{
	if (!CompatibilityManager::isWine())
	{
		int nItem = ctrlHubs.insertItem(info, 0);
		if (nItem >= 0)
		{
			ctrlHubs.SetCheckState(nItem, (ctrlHubs.GetCheckState(0) ? info->m_is_op : true));
			ctrlHubs.SetColumnWidth(0, LVSCW_AUTOSIZE);
		}
	}
}

void SearchFrame::onHubChanged(HubInfo* info)
{
	if (!CompatibilityManager::isWine())
	{
		int nItem = 0;
		const int n = ctrlHubs.GetItemCount();
		for (; nItem < n; nItem++)
		{
			if (ctrlHubs.getItemData(nItem)->url == info->url)
				break;
		}
		if (nItem == n)
			return;
			
		delete ctrlHubs.getItemData(nItem);
		ctrlHubs.SetItemData(nItem, (DWORD_PTR)info);
		ctrlHubs.updateItem(nItem);
		
		if (ctrlHubs.GetCheckState(0))
			ctrlHubs.SetCheckState(nItem, info->m_is_op);
			
		ctrlHubs.SetColumnWidth(0, LVSCW_AUTOSIZE);
	}
}

void SearchFrame::onHubRemoved(HubInfo* info)
{
	if (isClosedOrShutdown())
		return;
	if (!CompatibilityManager::isWine())
	{
		int nItem = 0;
		const int n = ctrlHubs.GetItemCount();
		for (; nItem < n; nItem++)
		{
			if (ctrlHubs.getItemData(nItem)->url == info->url)
				break;
		}
		
		delete info;
		
		if (nItem == n)
			return;
			
		delete ctrlHubs.getItemData(nItem);
		ctrlHubs.DeleteItem(nItem);
		ctrlHubs.SetColumnWidth(0, LVSCW_AUTOSIZE);
	}
}

LRESULT SearchFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ctrlResults.forEachSelected(&SearchInfo::getList);
	return 0;
}

LRESULT SearchFrame::onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ctrlResults.forEachSelected(&SearchInfo::browseList);
	return 0;
}

void SearchFrame::clearPausedResults()
{
#ifdef FLYLINK_DC_USE_PAUSED_SEARCH
	for (auto i : m_pausedResults)
	{
		delete *i;
		check_delete(*i);
	}
	m_pausedResults.clear();
#endif
}

LRESULT SearchFrame::onPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_running)
	{
		m_running = true;
		
#ifdef FLYLINK_DC_USE_PAUSED_SEARCH
		// readd all results which came during pause state
		while (!m_pausedResults.empty())
		{
			// start from the end because erasing front elements from vector is not efficient
			addSearchResult(m_pausedResults.back());
			m_pausedResults.pop_back();
		}
#endif
		// update controls texts
		ctrlStatus.SetText(3, (Util::toStringW(ctrlResults.GetItemCount()) + _T(' ') + TSTRING(FILES)).c_str());
		ctrlPauseSearch.SetWindowText(CTSTRING(PAUSE));
	}
	else
	{
		m_running = false;
		ctrlPauseSearch.SetWindowText(CTSTRING(CONTINUE_SEARCH));
	}
	return 0;
}

LRESULT SearchFrame::onItemChangedHub(int /* idCtrl */, LPNMHDR pnmh, BOOL& /* bHandled */)
{
	if (!CompatibilityManager::isWine())
	{
		const NMLISTVIEW* lv = (NMLISTVIEW*)pnmh;
		if (lv->iItem == 0 && (lv->uNewState ^ lv->uOldState) & LVIS_STATEIMAGEMASK)
		{
			if (((lv->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1)
			{
				for (int iItem = 0; (iItem = ctrlHubs.GetNextItem(iItem, LVNI_ALL)) != -1;)
				{
					const HubInfo* client = ctrlHubs.getItemData(iItem);
					if (!client->m_is_op)
						ctrlHubs.SetCheckState(iItem, false);
				}
			}
		}
	}
	return 0;
}

LRESULT SearchFrame::onPurge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tooltip.Activate(FALSE);
	ctrlSearchBox.ResetContent();
	g_lastSearches.clear();
	CFlylinkDBManager::getInstance()->clean_registry(e_SearchHistory, 0);
	MainFrame::updateQuickSearches(true);//[+]IRainman
	if (!BOOLSETTING(POPUPS_DISABLED))
	{
		m_tooltip.Activate(TRUE);
	}
	return 0;
}

LRESULT SearchFrame::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string data;
	// !SMT!-UI: copy several rows
	int i = -1;
	while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
	{
		const SearchInfo* l_si = ctrlResults.getItemData(i);
		const SearchResult &sr = l_si->m_sr;
		string sCopy;
		switch (wID)
		{
			case IDC_COPY_NICK:
				sCopy = sr.getUser()->getLastNick();
				break;
			case IDC_COPY_FILENAME:
				if (sr.getType() == SearchResult::TYPE_FILE || sr.getType() == SearchResult::TYPE_TORRENT_MAGNET)
					sCopy = Util::getFileName(sr.getFile());
				else
					sCopy = Util::getLastDir(sr.getFile());
				break;
			case IDC_COPY_PATH:
				sCopy = Util::getFilePath(sr.getFile());
				break;
			case IDC_COPY_SIZE:
				sCopy = Util::formatBytes(sr.getSize());
				break;
			case IDC_COPY_HUB_URL:
				sCopy = Util::formatDchubUrl(sr.getHubUrl());
				break;
			case IDC_COPY_LINK:
			case IDC_COPY_FULL_MAGNET_LINK:
				if (sr.getType() == SearchResult::TYPE_FILE)
					sCopy = Util::getMagnet(sr.getTTH(), sr.getFileName(), sr.getSize());
				if (wID == IDC_COPY_FULL_MAGNET_LINK && !sr.getHubUrl().empty())
					sCopy += "&xs=" + Util::formatDchubUrl(sr.getHubUrl());
				break;
			case IDC_COPY_WMLINK: // !SMT!-UI
				if (sr.getType() == SearchResult::TYPE_FILE)
					sCopy = Util::getWebMagnet(sr.getTTH(), sr.getFileName(), sr.getSize());
				break;
			case IDC_COPY_TTH:
				if (sr.getType() == SearchResult::TYPE_FILE)
					sCopy = sr.getTTH().toBase32();
				else if (sr.getType() == SearchResult::TYPE_TORRENT_MAGNET)
					sCopy = sr.getTorrentMagnet();
				break;
			case IDC_COPY_TORRENT_DATE:
				sCopy = sr.m_date;
				break;
			case IDC_COPY_TORRENT_COMMENT:
				sCopy = Util::toString(sr.m_comment);
				break;
			case IDC_COPY_TORRENT_URL:
				sCopy = sr.m_url;
				break;
			case IDC_COPY_TORRENT_PAGE:
				sCopy = Util::toString(sr.m_torrent_page);
				break;
			default:
				dcdebug("SEARCHFRAME DON'T GO HERE\n");
				return 0;
		}
		if (!sCopy.empty())
		{
			if (data.empty())
				data = sCopy;
			else
				data = data + "\r\n" + sCopy;
		}
	}
	if (!data.empty())
	{
		WinUtil::setClipboard(data);
	}
	return 0;
}

LRESULT SearchFrame::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW cd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh);
	
	switch (cd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			ctrlResultsFocused = ctrlResults.m_hWnd == GetFocus();
			return CDRF_NOTIFYITEMDRAW;
			
		case CDDS_ITEMPREPAINT:
		{
			cd->clrText = Colors::g_textColor;
			cd->clrTextBk = Colors::g_bgColor;
			SearchInfo* si = reinterpret_cast<SearchInfo*>(cd->nmcd.lItemlParam);
			if (si)
			{
				si->calcImageIndex();
				if (!si->m_is_torrent)
				{
					si->m_sr.checkTTH();
					// FIXME: more colors
					if (si->m_sr.flags & SearchResult::FLAG_SHARED)
						cd->clrTextBk = SETTING(DUPE_COLOR);
					/* FIXME
					else
					if (si->m_sr.flags & SearchResult::FLAG_QUEUED)
						cd->clrTextBk = SETTING(DUPE_EX3_COLOR);
					*/
					if (!si->columns[COLUMN_FLY_SERVER_RATING].empty())
						cd->clrTextBk = OperaColors::brightenColor(cd->clrTextBk, -0.02f);
					si->m_sr.calcHubName();
					if (si->m_location.isNew())
					{
						auto ip = si->getText(COLUMN_IP);
						if (!ip.empty())
						{
							const string l_str_ip = Text::fromT(ip);
							si->m_location = Util::getIpCountry(l_str_ip);
#ifdef FLYLINKDC_USE_LASTIP_AND_USER_RATIO
							if (si->m_is_flush_ip_to_sqlite == false &&
							        m_storeIP &&
							        si->getUser() && // https://drdump.com/Problem.aspx?ProblemID=364805
							        si->getUser()->getHubID())
							{
								si->m_is_flush_ip_to_sqlite = true;
								boost::system::error_code ec;
								const auto l_ip = boost::asio::ip::address_v4::from_string(l_str_ip, ec);
								dcassert(!ec);
								si->getUser()->setIP(l_ip, true);
							}
#endif
						}
					}
				}
			}
			return /*CDRF_NEWFONT | */CDRF_NOTIFYSUBITEMDRAW;
		}
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			SearchInfo* si = reinterpret_cast<SearchInfo*>(cd->nmcd.lItemlParam);
			if (!si)
				return CDRF_DODEFAULT;
			int column = ctrlResults.findColumn(cd->iSubItem);
			if (!si->m_is_torrent)
			{
				if (column == COLUMN_P2P_GUARD)
				{
					CRect rc;
					ctrlResults.GetSubItemRect((int)cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, rc);
					ctrlResults.SetItemFilled(cd, rc, cd->clrText, cd->clrText);
					si->m_sr.calcP2PGuard();
					const tstring l_value = si->getText(column);
					if (!l_value.empty())
					{
						LONG top = rc.top + (rc.Height() - 15) / 2;
						if ((top - rc.top) < 2)
							top = rc.top + 1;
						const POINT ps = { rc.left, top };
						g_userStateImage.Draw(cd->nmcd.hdc, 3, ps);
						::ExtTextOut(cd->nmcd.hdc, rc.left + 6 + 17, rc.top + 2, ETO_CLIPPED, rc, l_value.c_str(), l_value.length(), NULL);
					}
					return CDRF_SKIPDEFAULT;
				}
#ifdef FLYLINKDC_USE_ANTIVIRUS_DB
				else if (column == COLUMN_ANTIVIRUS)
				{
					CRect rc;
					ctrlResults.GetSubItemRect((int)cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, rc);
					ctrlResults.SetItemFilled(cd, rc, cd->clrText, cd->clrText);
					const tstring& l_value = si->getText(column);
					if (!l_value.empty())
					{
						LONG top = rc.top + (rc.Height() - 15) / 2;
						if ((top - rc.top) < 2)
							top = rc.top + 1;
						const POINT ps = { rc.left, top };
						g_userStateImage.Draw(cd->nmcd.hdc, 3, ps);
						::ExtTextOut(cd->nmcd.hdc, rc.left + 6 + 17, rc.top + 2, ETO_CLIPPED, rc, l_value.c_str(), l_value.length(), NULL);
					}
					return CDRF_SKIPDEFAULT;
				}
#endif
				if (column == COLUMN_LOCATION)
				{
					const auto& location = si->m_location;
					if (location.isKnown())
					{
						CustomDrawHelpers::drawLocation(ctrlResults, ctrlResultsFocused, cd, location);
						return CDRF_SKIPDEFAULT;
					}
					return CDRF_DODEFAULT;
				}
				
#ifdef SCALOLAZ_MEDIAVIDEO_ICO
				if (column == COLUMN_MEDIA_XY)
				{
					unsigned width, height;
					const tstring& text = si->columns[COLUMN_MEDIA_XY];
					if (CustomDrawHelpers::parseVideoResString(text, width, height) &&
					    CustomDrawHelpers::drawVideoResIcon(ctrlResults, ctrlResultsFocused, cd, text, width, height))
						return CDRF_SKIPDEFAULT;
				}
#endif //SCALOLAZ_MEDIAVIDEO_ICO
			}
			else // torrent
			{
				if (column == COLUMN_TTH)
				{
					CRect rc;
					ctrlResults.GetSubItemRect((int)cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, rc);
					ctrlResults.SetItemFilled(cd, rc, cd->clrText, cd->clrText);
					const tstring l_value = si->getText(column);
					if (!l_value.empty())
					{
						LONG top = rc.top + (rc.Height() - 15) / 2;
						if ((top - rc.top) < 2)
							top = rc.top + 1;
						const POINT ps = { rc.left, top };
						if (si->m_sr.m_tracker_index >= 0)
						{
							g_trackerImage.Draw(cd->nmcd.hdc, si->m_sr.m_tracker_index, ps);
						}
						::ExtTextOut(cd->nmcd.hdc, rc.left + 6 + 17, rc.top + 2, ETO_CLIPPED, rc, l_value.c_str(), l_value.length(), NULL);
					}
					return CDRF_SKIPDEFAULT;
				}
			}
			
			return CDRF_DODEFAULT;
		}
		default:
			return CDRF_DODEFAULT;
	}
//	return CDRF_DODEFAULT; /// all case have a return, this return never run.
}

LRESULT SearchFrame::onFilterChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (!BOOLSETTING(FILTER_ENTER) || wParam == VK_RETURN)
	{
		WinUtil::getWindowText(ctrlFilter, filter);
		Text::makeLower(filter);
		updateSearchList();
	}
	bHandled = false;
	return 0;
}
#ifdef FLYLINKDC_USE_ADVANCED_GRID_SEARCH

void SearchFrame::filtering(SearchInfo* si /*= nullptr */)
{
	if (!filterAllowRunning)
	{
		return;
	}
	//CFlyLock(cs);
	updatePrevTimeFilter();
	//TODO boost::thread t(std::bind(&SearchFrame::updateSearchListSafe, this, si));
}

bool SearchFrame::doFilter(WPARAM wParam)
{
	if (BOOLSETTING(FILTER_ENTER) && wParam != VK_RETURN)
	{
		return false;
	}
	
	if (GetKeyState(VK_CONTROL) &  0x8000)
	{
		if (wParam != 0x56) // ctrl+v
		{
			return false;
		}
	}
	else
	{
		switch (wParam)
		{
			case VK_LEFT:
			case VK_RIGHT:
			case VK_CONTROL/*single*/:
			case VK_SHIFT:
			{
				return false;
			}
		}
	}
	return true;
}

LRESULT SearchFrame::onKeyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
		{
			bHandled = FALSE;
			return 0;
		}
		case WM_KEYUP:
		{
			bHandled = TRUE;
			if (!doFilter(wParam))
			{
				return 0;
			}
			// TODO filtering();
			return 0;
		}
		case WM_CHAR:
		{
			bHandled = FALSE;
			return 0;
		}
	}
	bHandled = FALSE;
	return 0;
}

LRESULT SearchFrame::onGridItemClick(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	switch (ctrlGridFilters.GetSelectedColumn())
	{
		case FGC_ADD:
		{
			insertIntofilter(ctrlGridFilters);
			return 0;
		}
		case FGC_REMOVE:
		{
			if (ctrlGridFilters.GetItemCount() < 2)
			{
				ctrlGridFilters.GetProperty(0, FGC_FILTER)->SetValue(CComVariant(_T("")));
				return 0;
			}
			ctrlGridFilters.DeleteItem(ctrlGridFilters.GetSelectedIndex());
			return 1;
		}
	}
	return 0;
}

LRESULT SearchFrame::onGridItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	updateSearchListSafe();
	return 0;
}

void SearchFrame::updateSearchListSafe(SearchInfo* si)
{
	//CFlyLock(cs);
	try
	{
		updateSearchList(si);
		return;
	}
	catch (...)
	{
		//TODO:
		dcdebug("updateSearchList corrupt");
	}
}

#endif

bool SearchFrame::parseFilter(FilterModes& mode, int64_t& size)
{
	tstring::size_type start = tstring::npos;
	tstring::size_type end = tstring::npos;
	int64_t multiplier = 1;
	
	if (filter.compare(0, 2, _T(">="), 2) == 0)
	{
		mode = GREATER_EQUAL;
		start = 2;
	}
	else if (filter.compare(0, 2, _T("<="), 2) == 0)
	{
		mode = LESS_EQUAL;
		start = 2;
	}
	else if (filter.compare(0, 2, _T("=="), 2) == 0)
	{
		mode = EQUAL;
		start = 2;
	}
	else if (filter.compare(0, 2, _T("!="), 2) == 0)
	{
		mode = NOT_EQUAL;
		start = 2;
	}
	else if (filter[0] == _T('<'))
	{
		mode = LESS;
		start = 1;
	}
	else if (filter[0] == _T('>'))
	{
		mode = GREATER;
		start = 1;
	}
	else if (filter[0] == _T('='))
	{
		mode = EQUAL;
		start = 1;
	}
	
	if (start == tstring::npos)
		return false;
	if (filter.length() <= start)
		return false;
		
	if ((end = filter.find(_T("tib"))) != tstring::npos)
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	else if ((end = filter.find(_T("gib"))) != tstring::npos)
		multiplier = 1024 * 1024 * 1024;
	else if ((end = filter.find(_T("mib"))) != tstring::npos)
		multiplier = 1024 * 1024;
	else if ((end = filter.find(_T("kib"))) != tstring::npos)
		multiplier = 1024;
	else if ((end = filter.find(_T("tb"))) != tstring::npos)
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	else if ((end = filter.find(_T("gb"))) != tstring::npos)
		multiplier = 1000 * 1000 * 1000;
	else if ((end = filter.find(_T("mb"))) != tstring::npos)
		multiplier = 1000 * 1000;
	else if ((end = filter.find(_T("kb"))) != tstring::npos)
		multiplier = 1000;
	else if ((end = filter.find(_T('b'))) != tstring::npos)
		multiplier = 1;
	
	if (end == tstring::npos)
	{
		end = filter.length();
	}
	
	const tstring tmpSize = filter.substr(start, end - start);
	size = static_cast<int64_t>(Util::toDouble(Text::fromT(tmpSize)) * multiplier);
	
	return true;
}

bool SearchFrame::matchFilter(const SearchInfo* si, int sel, bool doSizeCompare, FilterModes mode, int64_t size)
{
	if (filter.empty()) return true;
	if (doSizeCompare)
	{
		bool insert = true;
		switch (mode)
		{
			case EQUAL:
				insert = (size == si->m_sr.getSize());
				break;
			case GREATER_EQUAL:
				insert = (size <= si->m_sr.getSize());
				break;
			case LESS_EQUAL:
				insert = (size >= si->m_sr.getSize());
				break;
			case GREATER:
				insert = (size < si->m_sr.getSize());
				break;
			case LESS:
				insert = (size > si->m_sr.getSize());
				break;
			case NOT_EQUAL:
				insert = (size != si->m_sr.getSize());
				break;
		}
		return insert;
	}
	tstring s = si->getText(static_cast<uint8_t>(sel));
	Text::makeLower(s);
	return s.find(filter) != tstring::npos;
}

void SearchFrame::set_tree_item_status(const SearchInfo* p_si)
{
	dcassert(ctrlResults.findItem(p_si) == -1);
	dcassert(!closed);
	const int k = ctrlResults.insertItem(p_si, I_IMAGECALLBACK);
	
	const vector<SearchInfo*>& children = ctrlResults.findChildren(p_si->getGroupCond());
	if (!children.empty())
	{
		if (p_si->collapsed)
		{
			ctrlResults.SetItemState(k, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		}
		else
		{
			ctrlResults.SetItemState(k, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
		}
	}
	else
	{
		ctrlResults.SetItemState(k, INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);
	}
}
void SearchFrame::clear_tree_filter_contaners()
{
	m_top_subitem_map.clear();
	m_category_map.clear();
	m_tree_ext_map.clear();
	m_tree_sub_torrent_map.clear();
	{
		CFlyLock(m_filter_map_cs);
		m_filter_map.clear();
	}
	m_tree_type.clear();
	m_CurrentTreeItem = nullptr;
	m_OldTreeItem = nullptr;
	m_RootTreeItem = nullptr;
	m_RootVirusTreeItem = nullptr;
	m_RootTorrentRSSTreeItem = nullptr;
	m_24HTopTorrentTreeItem = nullptr;
	m_is_expand_tree = false;
	m_is_expand_sub_tree = false;
	m_ctrlSearchFilterTree.DeleteAllItems();
	{
		CFlyFastLock(m_si_set_cs);
		for (auto i = m_si_set.begin(); i != m_si_set.end(); ++i)
		{
			delete *i;
		}
		m_si_set.clear();
	}
}

void SearchFrame::updateSearchList(SearchInfo* p_si)
{
	dcassert(!closed);
	int64_t size = -1;
	FilterModes mode = NONE;
	const int sel = ctrlFilterSel.GetCurSel();
	bool doSizeCompare = sel == COLUMN_SIZE && parseFilter(mode, size);
	if (p_si)
	{
		if (!matchFilter(p_si, sel, doSizeCompare, mode, size))
		{
			ctrlResults.deleteItem(p_si);
		}
	}
	else
	{
		CFlyLock(m_filter_map_cs);
		const auto& l_virus = m_filter_map[m_RootVirusTreeItem];
		CLockRedraw<> l_lock_draw(ctrlResults);
		ctrlResults.DeleteAllItems();
		for (auto i = ctrlResults.getParents().cbegin(); i != ctrlResults.getParents().cend(); ++i)
		{
			bool l_is_virus = false;
			SearchInfo* l_si = (*i).second.parent;
			for (auto j : l_virus)
			{
				if (j.first == l_si)
				{
					l_is_virus = true;
					break;
				}
			}
			if (l_is_virus == false)
			{
				l_si->collapsed = true;
				if (matchFilter(l_si, sel, doSizeCompare, mode, size))
				{
					set_tree_item_status(l_si);
				}
			}
		}
	}
}

LRESULT SearchFrame::onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	WinUtil::getWindowText(ctrlFilter, filter);
	updateSearchList();
	bHandled = false;
	return 0;
}

// This and related changes taken from apex-sppedmod by SMT
LRESULT SearchFrame::onEditChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
//+BugMaster: new options; small optimization
	const unsigned sz = ctrlSearchBox.GetWindowTextLength();
	tstring searchString;
	if (sz == 39)
	{
		WinUtil::getWindowText(ctrlSearchBox, searchString);
		if (isTTH(searchString))
		{
			ctrlFiletype.SetCurSel(ctrlFiletype.FindStringExact(0, _T("TTH")));
			m_lastFindTTH = true;
		}
	}
	else
	{
		WinUtil::getWindowText(ctrlSearchBox, searchString);
		if (!searchString.empty())
		{
			if (Util::isMagnetLink(searchString))
			{
				const tstring::size_type xt = searchString.find(L"xt=urn:tree:tiger:");
				if (xt != tstring::npos && xt + 18 + 39 <= searchString.size() //[!]FlylinkDC++ Team. fix crash: "magnet:?xt=urn:tree:tiger:&xl=3451326464&dn=sr-r3ts12.iso"
				   )
				{
					searchString = searchString.substr(xt + 18, 39);
					if (isTTH(searchString))
					{
						ctrlSearchBox.SetWindowText(searchString.c_str());
						ctrlFiletype.SetCurSel(ctrlFiletype.FindStringExact(0, _T("TTH")));
						m_lastFindTTH = true;
						return 0;
					}
				}
			}
		}
		if (m_lastFindTTH)
		{
			//� ������� ������ ����� ������ �������, ��� ��� ������� ���� ������������� ������������� ��� ����� TTH. �� ���� ����� ����� �����������
			//����� �� �����, �� ���������� ������� TTH ������� --> �������������� ��������� ��� ������������ � �������� TTH ������� ������� "�����".
			ctrlFiletype.SetCurSel(ctrlFiletype.FindStringExact(0, CTSTRING(ANY)));
			m_lastFindTTH = false;
		}
	}
	return 0;
}
//-BugMaster: new options; small optimization

void SearchFrame::on(SettingsManagerListener::Repaint)
{
	dcassert(!ClientManager::isBeforeShutdown());
	if (!ClientManager::isBeforeShutdown())
	{
		if (ctrlResults.isRedraw())
		{
			ctrlResults.setFlickerFree(Colors::g_bgBrush);
			ctrlHubs.SetBkColor(Colors::g_bgColor);
			ctrlHubs.SetTextBkColor(Colors::g_bgColor);
			ctrlHubs.SetTextColor(Colors::g_textColor);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
}

LRESULT SearchFrame::onMarkAsDownloaded(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i = -1;
	
	while ((i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1)
	{
		const SearchInfo* si = ctrlResults.getItemData(i);
		const SearchResult& sr = si->m_sr;
		if (sr.getType() == SearchResult::TYPE_FILE)
		{
#if 0 // FIXME
			CFlylinkDBManager::getInstance()->push_download_tth(sr.getTTH());
#endif
			ctrlResults.updateItem(si);
		}
	}
	
	return 0;
}

void SearchFrame::speak(Speakers s, const Client* aClient)
{
	if (!isClosedOrShutdown())
	{
		HubInfo * hubInfo = new HubInfo(Text::toT(aClient->getHubUrl()), Text::toT(aClient->getHubName()), aClient->getMyIdentity().isOp());
		safe_post_message(*this, s, hubInfo);
	}
}

void SearchFrame::updateResultCount()
{
	const size_t totalResult = resultsCount
#ifdef FLYLINK_DC_USE_PAUSED_SEARCH
		+ m_pausedResults.size()
#endif
	;
	ctrlStatus.SetText(3, (Util::toStringW(totalResult) + _T('/') + Util::toStringW(resultsCount) + _T(' ') + WSTRING(FILES)).c_str());
	ctrlStatus.SetText(4, (Util::toStringW(droppedResults) + _T(' ') + TSTRING(FILTERED)).c_str());
	needUpdateResultCount = false;
	#if 0 // not used
	setCountMessages(resultsCount);
	#endif
}

void SearchFrame::updateStatusLine(uint64_t tick)
{
	auto delta = m_searchEndTime - m_searchStartTime;
	uint64_t percent = delta ? (tick - m_searchStartTime) * 100 / delta : 0;
	bool searchDone = percent >= 100;
	if (searchDone)
	{
		statusLine = m_target + _T(" - ") + TSTRING(DONE);
		ctrlStatus.SetText(2, CTSTRING(DONE));
	}
	else
	{
		statusLine = TSTRING(SEARCHING_FOR) + _T(' ') + m_target + _T(" ... ") + Util::toStringW(percent) + _T("%");
		ctrlStatus.SetText(2, Util::formatSecondsW((m_searchEndTime - tick) / 1000).c_str());
	}
	SetWindowText(statusLine.c_str());
	::InvalidateRect(m_hWndStatusBar, NULL, TRUE);
}

void SearchFrame::showPortStatus()
{
	if (g_DisableTestPort) return;
	string reflectedAddress;
	int port;
	int state = g_portTest.getState(PortTest::PORT_UDP, port, &reflectedAddress);
	if (state == portStatus && currentReflectedAddress == reflectedAddress) return;
	tstring newText;
	switch (state)
	{
		case PortTest::STATE_RUNNING:
			ctrlUDPMode.SetIcon(g_UDPWaitIcon);
			newText = CTSTRING(UDP_PORT_TEST_WAIT);
			break;

		case PortTest::STATE_FAILURE:
			ctrlUDPMode.SetIcon(g_UDPFailIcon);
			newText = CTSTRING(UDP_PORT_TEST_FAILED);
			break;

		case PortTest::STATE_SUCCESS:
			ctrlUDPMode.SetIcon(g_UDPOkIcon);
			newText = CTSTRING(UDP_PORT_TEST_OK);
			if (!reflectedAddress.empty())
			{
				newText += _T(" (");
				newText += Text::toT(reflectedAddress);
				newText += _T(")");
			}
			break;

		default:
			return;
	}
	portStatus = state;
	currentReflectedAddress = std::move(reflectedAddress);
	if (newText != portStatusText)
	{
		portStatusText = std::move(newText);
		ctrlUDPTestResult.SetWindowText(portStatusText.c_str());
	}
	ClientManager::infoUpdated(true);
}

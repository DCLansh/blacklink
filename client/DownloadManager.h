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

#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H
#define DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H

#include "DownloadManagerListener.h"
#include "UserConnectionListener.h"
#include "QueueItem.h"
#include "TimerManager.h"
#include "Singleton.h"

#ifdef FLYLINKDC_USE_TORRENT
#include "libtorrent/session_types.hpp"
#include "libtorrent/torrent_handle.hpp"
namespace libtorrent { class session; }
#endif

/**
 * Singleton. Use its listener interface to update the download list
 * in the user interface.
 */
//typedef boost::unordered_map<UserPtr, UserConnection*, User::Hash> IdlersMap;
typedef std::vector<UserConnection*> UserConnectionList;

class DownloadManager : public Speaker<DownloadManagerListener>,
	private UserConnectionListener, private TimerManagerListener,
	public Singleton<DownloadManager>
{
#ifdef FLYLINKDC_USE_TORRENT
		std::unique_ptr<libtorrent::session> m_torrent_session;
		void onTorrentAlertNotify(libtorrent::session* p_torrent_sesion);
		void select_files(const libtorrent::torrent_handle& p_torrent_handle);
		std::atomic<int> m_torrent_resume_count = { 0 };
		std::atomic<int> m_torrent_rename_count = { 0 };
		std::unordered_set<libtorrent::torrent_handle> m_torrents;
	public:
		void init_torrent(bool p_is_force = false);
		void shutdown_torrent();
		bool add_torrent_file(const tstring& p_torrent_path, const tstring& p_torrent_url);
		bool remove_torrent_file(const libtorrent::sha1_hash& p_sha1, libtorrent::remove_flags_t p_options);
		bool pause_torrent_file(const libtorrent::sha1_hash& p_sha1, bool p_is_resume);
		bool set_file_priority(const libtorrent::sha1_hash& p_sha1, const CFlyTorrentFileArray& p_files,
		                       const std::vector<libtorrent::download_priority_t>& p_file_priority, const std::string& p_save_path);
		int listen_torrent_port();
		int ssl_listen_torrent_port();
		void fire_added_torrent(const libtorrent::sha1_hash& p_sha1);
		std::string get_torrent_name(const libtorrent::sha1_hash& p_sha1);
		std::string get_torrent_magnet(const libtorrent::sha1_hash& p_sha1);
#endif
		
	public:
		/** @internal */
		void addConnection(UserConnection* p_conn);
		static void checkIdle(const UserPtr& user);
		
		/** @internal */
		static void abortDownload(const string& target);
		
		/** @return Running average download speed in Bytes/s */
		static int64_t getRunningAverage() { return g_runningAverage; }
		static void setRunningAverage(int64_t avg) { g_runningAverage = avg; }
		
		static size_t getDownloadCount();
		
		static bool isStartDownload(QueueItem::Priority prio);
		static bool checkFileDownload(const UserPtr& aUser);
		void onData(UserConnection*, const uint8_t*, size_t) noexcept;

	private:
		static std::unique_ptr<RWLock> g_csDownload;
		static DownloadList g_download_map;
		static UserConnectionList g_idlers;
		static void remove_idlers(UserConnection* source);
		
		static int64_t g_runningAverage;
		
		void removeConnection(UserConnection* conn);
		static void removeDownload(const DownloadPtr& aDownload);
		void fileNotAvailable(UserConnection* source);
		void noSlots(UserConnection* source, const string& param = Util::emptyString);
		
		void failDownload(UserConnection* source, const string& reason);
		
		friend class Singleton<DownloadManager>;
		
		DownloadManager();
		~DownloadManager();
		
		void checkDownloads(UserConnection* conn);
		void startData(UserConnection* source, int64_t start, int64_t newSize, bool z);
		void endData(UserConnection* source);
		
		void onFailed(UserConnection* source, const string& error);
		
		// UserConnectionListener
		void on(Failed, UserConnection* source, const string& error) noexcept override
		{
			onFailed(source, error);
		}
		void on(ProtocolError, UserConnection* source, const string& error) noexcept override
		{
			onFailed(source, error);
		}
		void on(MaxedOut, UserConnection*, const string& param) noexcept override;
		void on(FileNotAvailable, UserConnection*) noexcept override;
		void on(ListLength, UserConnection* source, const string& listLength) noexcept override;
		void on(Updated, UserConnection*) noexcept override;
		
		void on(AdcCommand::SND, UserConnection*, const AdcCommand&) noexcept override;
		void on(AdcCommand::STA, UserConnection*, const AdcCommand&) noexcept override;
		
		// TimerManagerListener
		void on(TimerManagerListener::Second, uint64_t tick) noexcept override;
		void on(CheckUserIP, UserConnection*) noexcept override;
};

#endif // !defined(DOWNLOAD_MANAGER_H)

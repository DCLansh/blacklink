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

#ifndef DCPLUSPLUS_DCPP_HASH_MANAGER_H
#define DCPLUSPLUS_DCPP_HASH_MANAGER_H

#include "Semaphore.h"
#include "TimerManager.h"
#include "Streams.h"
#include "HashManagerListener.h"

#ifdef RIP_USE_STREAM_SUPPORT_DETECTION
#include "FsUtils.h"
#endif

#define IRAINMAN_NTFS_STREAM_TTH

#define MAX_HASH_PROGRESS_VALUE 3000

class File;

class FileException;

class HashManager : public Singleton<HashManager>, public Speaker<HashManagerListener>,
	private TimerManagerListener
{
	public:
	
		HashManager()
		{
			TimerManager::getInstance()->addListener(this);
		}
		virtual ~HashManager() noexcept
		{
			TimerManager::getInstance()->removeListener(this);
			hasher.join();
		}
		
		void hashFile(int64_t fileID, const SharedFilePtr& file, const string& fileName, int64_t size)
		{
			hasher.hashFile(fileID, file, fileName, size);
		}
		
		void stopHashing(const string& baseDir)
		{
			hasher.stopHashing(baseDir);
		}
		void setThreadPriority(Thread::Priority p)
		{
			hasher.setThreadPriority(p);
		}
		
		/*
		void addTree(const string& aFileName, int64_t aTimeStamp, const TigerTree& tt, int64_t size)
		{
			hashDone(0, aFileName, aTimeStamp, tt, -1, false, size);
		}
		*/

		void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft)
		{
			hasher.getStats(curFile, bytesLeft, filesLeft);
		}
		
		/**
		 * Rebuild hash data file
		 */
		void rebuild()
		{
			hasher.scheduleRebuild();
		}
		
		void startup()
		{
			hasher.start(0, "HashManager");
		}
		
		void shutdown()
		{
			// [-] brain-ripper
			// Removed critical section - it caused deadlock on client exit:
			// "join" hangs on WaitForSingleObject(threadHandle, INFINITE),
			// and threadHandle thread hangs on this very critical section
			// in HashManager::hashDone
			//CFlyLock(cs); //[!]IRainman
			hasher.shutdown();
			hasher.join();
		}
		
		struct HashPauser
		{
				HashPauser();
				~HashPauser();
				
			private:
				bool resume;
		};
		
		/// @return whether hashing was already paused
		bool pauseHashing();
		void resumeHashing();
		bool isHashingPaused() const;
		
		
//[+] Greylink
#ifdef IRAINMAN_NTFS_STREAM_TTH
		class StreamStore   // greylink dc++: work with ntfs stream
		{
			public:
				static bool doLoadTree(const string& filePath, TigerTree& tree, int64_t fileSize, bool checkTimestamp) noexcept;
				bool loadTree(const string& p_filePath, TigerTree& p_Tree, int64_t p_aFileSize = -1);
				bool saveTree(const string& p_filePath, const TigerTree& p_Tree);// [+] IRainman const string& p_filePath
				void deleteStream(const string& p_filePath);//[+] IRainman
				
#ifdef RIP_USE_STREAM_SUPPORT_DETECTION
				void SetFsDetectorNotifyWnd(HWND hWnd);
				void OnDeviceChange(LPARAM lParam, WPARAM wParam)
				{
					m_FsDetect.OnDeviceChange(lParam, wParam);
				}
#endif
			private:
#ifndef RIP_USE_STREAM_SUPPORT_DETECTION
				static std::unordered_set<char> g_error_tth_stream; //[+]PPA
				static void addBan(const string& p_filePath)
				{
					if (p_filePath.size())
					{
						//CFlyFastLock(m_cs); // [!] IRainman fix.
						g_error_tth_stream.insert(p_filePath[0]);
					}
				}
				static bool isBan(const string& p_filePath)
				{
					//CFlyFastLock(m_cs); // [!] IRainman fix.
					return p_filePath.size() && g_error_tth_stream.find(p_filePath[0]) != g_error_tth_stream.end(); // [!] IRainman opt.
				}
				//FastCriticalSection m_cs; // [+] IRainman fix.
#endif
				
#ifdef RIP_USE_STREAM_SUPPORT_DETECTION
				// [+] brain-ripper
				// Detector if volume support streams
				FsUtils::CFsTypeDetector m_FsDetect;
#endif
		};
//[~] Greylink
		StreamStore m_streamstore;
#endif // IRAINMAN_NTFS_STREAM_TTH
		
#ifdef RIP_USE_STREAM_SUPPORT_DETECTION
		void SetFsDetectorNotifyWnd(HWND hWnd);
		void OnDeviceChange(LPARAM lParam, WPARAM wParam)
		{
			m_streamstore.OnDeviceChange(lParam, wParam);
		}
#endif
		
		// [+] brain-ripper
		// Temporarily change hash speed functional
		bool IsHashing()
		{
			return hasher.IsHashing();
		}
		size_t GetProgressValue()
		{
			return hasher.GetProgressValue();
		}
		uint64_t GetStartTime()
		{
			return hasher.GetStartTime();
		}
		void GetProcessedFilesAndBytesCount(int64_t &bytes, size_t &files)
		{
			hasher.GetProcessedFilesAndBytesCount(bytes, files);
		}
		static inline DWORD GetMaxProgressValue()
		{
			return Hasher::GetMaxProgressValue();
		}
		void EnableForceMinHashSpeed(int iMinHashSpeed)
		{
			hasher.EnableForceMinHashSpeed(iMinHashSpeed);
		}
		void DisableForceMinHashSpeed()
		{
			hasher.DisableForceMinHashSpeed();
		}
		int GetMaxHashSpeed()
		{
			return hasher.GetMaxHashSpeed();
		}
		// [~] brain-ripper
		
		
	private:
		class Hasher : public Thread, private CFlyStopThread
		{
			public:
				Hasher() : m_running(false), m_paused(0), m_rebuild(false), m_currentSize(0), m_fileID(0),
					m_CurrentBytesLeft(0), m_ForceMaxHashSpeed(0), dwMaxFiles(0), iMaxBytes(0), uiStartTime(0), m_last_error(0), m_last_error_overlapped(0) { }
					
				void hashFile(int64_t fileID, const SharedFilePtr& file, const string& fileName, int64_t size);
				
				/// @return whether hashing was already paused
				bool pause();
				void resume();
				bool isPaused() const;
				
				void stopHashing(const string& baseDir);
				virtual int run() override;
				bool fastHash(const string& fname, uint8_t* buf, unsigned p_buf_size, TigerTree& tth, int64_t& size, bool p_is_link);
				// [+] brain-ripper
				void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft)
				{
					CFlyFastLock(cs);
					curFile = m_fname;
					getBytesAndFileLeft(bytesLeft, filesLeft);
				}
				
				void signal()
				{
					if (m_paused)
					{
						m_hash_semaphore.signal(); // TODO ��� ����� ����� ����� ������� ������?
					}
					m_hash_semaphore.signal();
				}
				void shutdown()
				{
					stopThread();
					signal();
				}
				void scheduleRebuild()
				{
					m_rebuild = true;
					signal();
				}
				
				int GetMaxHashSpeed() const
				{
					return m_ForceMaxHashSpeed != 0 ? m_ForceMaxHashSpeed : SETTING(MAX_HASH_SPEED);
				}
			private:
				void getBytesAndFileLeft(int64_t& bytesLeft, size_t& filesLeft) const
				{
					filesLeft = w.size();
					if (m_running)
						filesLeft++;
						
					bytesLeft = m_currentSize + m_CurrentBytesLeft;
				}
			public:
				void EnableForceMinHashSpeed(int iMinHashSpeed)
				{
					m_ForceMaxHashSpeed = iMinHashSpeed;
				}
				void DisableForceMinHashSpeed()
				{
					m_ForceMaxHashSpeed = 0;
				}
				
				bool IsHashing() const
				{
					return m_running;
				}
				
				size_t GetProgressValue()
				{
					if (!m_running || dwMaxFiles == 0 || iMaxBytes == 0)
						return 0;
						
					int64_t bytesLeft;
					size_t filesLeft;
					{
						CFlyFastLock(cs);
						getBytesAndFileLeft(bytesLeft, filesLeft);
					}
					
					return static_cast<size_t>(MAX_HASH_PROGRESS_VALUE * ((static_cast<double>(dwMaxFiles - filesLeft + 1) / static_cast<double>(dwMaxFiles)) * (static_cast<double>(iMaxBytes - bytesLeft) / static_cast<double>(iMaxBytes))));
				}
				
				uint64_t GetStartTime() const
				{
					return IsHashing() ? uiStartTime : 0;
				}
				
				void GetProcessedFilesAndBytesCount(int64_t &bytes, size_t &files)
				{
					if (IsHashing())
					{
						int64_t bytesLeft;
						size_t filesLeft;
						{
							CFlyFastLock(cs);
							getBytesAndFileLeft(bytesLeft, filesLeft);
						}
						bytes = iMaxBytes >= bytesLeft ? iMaxBytes - bytesLeft : iMaxBytes;
						files = dwMaxFiles >= filesLeft ? dwMaxFiles - filesLeft : dwMaxFiles;
					}
					else
					{
						bytes = 0;
						files = 0;
					}
				}
				
				static DWORD GetMaxProgressValue()
				{
					return MAX_HASH_PROGRESS_VALUE;
				}
				
			private:
				// Case-sensitive (faster), it is rather unlikely that case changes, and if it does it's harmless.
				// map because it's sorted (to avoid random hash order that would create quite strange shares while hashing)
				struct HashTaskItem
				{
					int64_t fileSize;
					int64_t fileID;
					SharedFilePtr file;
					string dir;
				};
				void instantPause();
				typedef std::map<string, HashTaskItem> WorkMap;
				
				WorkMap w;
				mutable FastCriticalSection cs;
				Semaphore m_hash_semaphore;
				
				volatile bool m_running;
				int64_t m_paused;
				volatile bool m_rebuild;
				int64_t m_currentSize;
				int64_t m_fileID;
				SharedFilePtr m_filePtr;
				int m_ForceMaxHashSpeed;
				size_t dwMaxFiles;
				int64_t iMaxBytes;
				uint64_t uiStartTime;
				int64_t m_CurrentBytesLeft;
				string m_fname;
				DWORD m_last_error;
				DWORD m_last_error_overlapped;
		};
		
		friend class Hasher;
		
		void addFile(const string& p_file_name, int64_t p_time_stamp, const TigerTree& p_tth, int64_t p_size, class CFlyMediaInfo& p_out_media);

	private:
#ifdef IRAINMAN_NTFS_STREAM_TTH
		void addFileFromStream(const string& p_name, const TigerTree& p_TT, int64_t p_size);
#endif
		
		Hasher hasher;
		
		void hashDone(int64_t fileID, const SharedFilePtr& file, const string& aFileName, int64_t aTimeStamp, const TigerTree& tth, int64_t speed, bool isNTFS, int64_t Size);
		void doRebuild()
		{
			rebuild();
		}
};

#endif // !defined(HASH_MANAGER_H)

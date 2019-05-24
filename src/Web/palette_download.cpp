#include "palette_download.h"

#include "url_downloader.h"

#include <DecompressClass.h>
#include "Core/logger.h"
#include "Core/info.h"
#include "Core/interfaces.h"
#include "Overlay/Logger/ImGuiLogger.h"

#include <vector>
#include <sstream>

#pragma comment(lib, "DecompressLibrary.lib")

void GetPalettesFromArchive()
{
	g_imGuiLogger->Log("[system] Extracting 'palettes.tar.gz'...\n");

	CDecompressClass decompressObj;
	int successCount = 0;
	int failCount = 0;
	
	if (!decompressObj.OpenArchive(L"BBTAG_IM/Download/palettes.tar.gz"))
	{
		std::string pStr = CT2A((LPCTSTR)decompressObj.GetErrorText());
		g_imGuiLogger->Log("[error] 'BBTAG_IM/Download/palettes.tar.gz'could not be opened: %s\n", pStr.c_str());
		LOG(2, "ERROR, 'BBTAG_IM/Download/palettes.tar.gz'could not be opened: %s\n", pStr.c_str());
		return;
	}

	g_imGuiLogger->EnableLog(false);

	// if we opened ok, read each file from the archive
	for (int i = 0; i < decompressObj.GetCompressedFilesCount(); i++)
	{
		char* pszFileBuffer;
		int nFileLength;
		CString szFileName;
		BOOL fIsFile;

		decompressObj.GetArchiveFile(i, &pszFileBuffer, nFileLength, fIsFile, szFileName);

		if (fIsFile)
		{
			std::string fileName = CT2A((LPCTSTR)szFileName);

			if (strncmp(((IMPL_t*)pszFileBuffer)->header.filesig, "IMPL", 5) != 0)
			{
				g_imGuiLogger->EnableLog(true);
				g_imGuiLogger->Log("[error] 'palettes.tar.gz' contains non .impl file: '%s'\n", fileName.c_str());
				g_imGuiLogger->EnableLog(false);
				LOG(2, "ERROR, 'palettes.tar.gz' contains non .impl file: '%s'\n", fileName.c_str());
				continue;
			}

			PaletteManager::OverwriteIMPLDataPalName(fileName, ((IMPL_t*)pszFileBuffer)->paldata);
			if (g_interfaces.pPaletteManager->PushImplFileIntoVector(*((IMPL_t*)pszFileBuffer)))
				successCount++; 
			else
				failCount++;
		}
	}
	g_imGuiLogger->EnableLog(true);
	g_imGuiLogger->Log("[system] Total of %d palettes had been successfully extracted from 'palettes.tar.gz' (%d succeeded, %d failed)\n", 
		successCount + failCount, successCount, failCount);
}

std::vector<std::wstring> DownloadPaletteFileList(std::wstring wUrl)
{
	LOG(2, "DownloadPaletteFileList\n");
	g_imGuiLogger->Log("[system] Getting online palette list...\n");
	
	std::string data = DownloadUrl(wUrl);

	std::vector<std::wstring> links;

	if (strcmp(data.c_str(), "") != 0 && data.find("404: Not Found") == std::string::npos)
	{
		std::stringstream ss(data);
		std::string line;
		while (std::getline(ss, line, '\n'))
		{
			int pos = line.find('\r');
			if (pos != std::string::npos)
			{
				line.erase(pos);
			}

			links.push_back(std::wstring(line.begin(), line.end()));
		}
	}
	else
	{
		LOG(2, "DownloadPaletteFileList failed, no data received, or 404!!!\n");
		g_imGuiLogger->Log("[error] Failed to retrieve online palette list. No data, or 404\n");
	}

	return links;
}

void DownloadPaletteArchive()
{
	if (!g_interfaces.pPaletteManager->PaletteArchiveDownloaded())
	{
		std::wstring wUrl(MOD_LINK_PALETTES_ARCHIVE);

		char* downlBuf = 0;

		g_imGuiLogger->Log("[system] Downloading latest 'palettes.tar.gz'...\n");
		int res = DownloadUrlBinary(wUrl, (void**)&downlBuf);

		if (res > 0 && downlBuf)
		{
			g_imGuiLogger->Log("[system] Finished downloading 'palettes.tar.gz'\n");

			if (utils_WriteFile("BBTAG_IM/Download/palettes.tar.gz", downlBuf, res, true))
			{
				g_interfaces.pPaletteManager->PaletteArchiveDownloaded() = true;
			}
			else
			{
				g_imGuiLogger->Log("[error] 'BBTAG_IM/Download/palettes.tar.gz'could not be written\n");
				LOG(2, "ERROR, 'BBTAG_IM/Download/palettes.tar.gz'could not be written\n");
			}
		}
		SAFE_DELETE_ARRAY(downlBuf);
	}

	GetPalettesFromArchive();
}

void StartAsyncPaletteArchiveDownload()
{
	CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)DownloadPaletteArchive, nullptr, 0, nullptr));
}


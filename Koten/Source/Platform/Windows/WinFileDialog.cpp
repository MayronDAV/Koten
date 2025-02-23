#include "ktnpch.h"
#include "Koten/OS/FileDialog.h"

#include <Windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <vector>
#include <string>
#include <iostream>



namespace KTN
{
	FileDialogResult FileDialog::Open(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = p_FilterList.empty() ? "All Files\0*.*\0" : p_FilterList.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = p_DefaultPath.empty() ? nullptr : p_DefaultPath.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			p_OutPath = ofn.lpstrFile;
			return FileDialogResult::SUCCESS;
		}

		return FileDialogResult::CANCEL;
	}

	FileDialogResult FileDialog::OpenMultiple(const std::string& p_FilterList, const std::string& p_DefaultPath, std::vector<std::string>& p_OutPaths)
	{
		OPENFILENAMEA ofn;
		char szFile[1024] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = p_FilterList.empty() ? "All Files\0*.*\0" : p_FilterList.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = p_DefaultPath.empty() ? nullptr : p_DefaultPath.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			char* p = szFile;
			std::string directory = p;
			p += directory.size() + 1;

			while (*p)
			{
				std::string file = p;
				p_OutPaths.push_back(directory + "\\" + file);
				p += file.size() + 1;
			}

			if (p_OutPaths.empty())
			{
				p_OutPaths.push_back(szFile);
			}

			return FileDialogResult::SUCCESS;
		}

		return FileDialogResult::CANCEL;
	}

	FileDialogResult FileDialog::Save(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = p_FilterList.empty() ? "All Files\0*.*\0" : p_FilterList.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = p_DefaultPath.empty() ? nullptr : p_DefaultPath.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			p_OutPath = ofn.lpstrFile;
			return FileDialogResult::SUCCESS;
		}

		return FileDialogResult::CANCEL;
	}

	FileDialogResult FileDialog::PickFolder(const std::string& p_DefaultPath, std::string& p_OutPath)
	{
		BROWSEINFOA bi;
		ZeroMemory(&bi, sizeof(bi));
		bi.hwndOwner = nullptr;
		bi.pszDisplayName = new char[MAX_PATH];
		bi.lpszTitle = "Selecione uma pasta";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

		if (!p_DefaultPath.empty())
		{
			bi.lParam = reinterpret_cast<LPARAM>(p_DefaultPath.c_str());
			bi.lpfn = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) -> int {
				if (uMsg == BFFM_INITIALIZED)
				{
					SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
				}
				return 0;
				};
		}

		LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
		if (pidl != nullptr)
		{
			char path[MAX_PATH];
			if (SHGetPathFromIDListA(pidl, path))
			{
				p_OutPath = path;
				delete[] bi.pszDisplayName;
				return FileDialogResult::SUCCESS;
			}

			CoTaskMemFree(pidl);
		}

		delete[] bi.pszDisplayName;
		return FileDialogResult::CANCEL;
	}
} // namespace KTN
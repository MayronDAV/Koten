#include "ktnpch.h"
#include "Koten/OS/FileDialog.h"

// lib
#include <gtk/gtk.h>

// std
#include <iostream>



namespace KTN
{
	FileDialogResult FileDialog::Open(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath) 
	{
		KTN_PROFILE_FUNCTION_LOW();

		FileDialogResult result = FileDialogResult::CANCEL;
		
		if (!gtk_init_check(nullptr, nullptr)) {
			KTN_CORE_ERROR("GTK initialization failed!");
			return FileDialogResult::FAILED;
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Open File",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"_Cancel", GTK_RESPONSE_CANCEL,
			"_Open", GTK_RESPONSE_ACCEPT,
			nullptr);

		if (!p_DefaultPath.empty()) {
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());
		}

		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		if (response == GTK_RESPONSE_ACCEPT) {
			char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			p_OutPath = filename;
			g_free(filename);
			result = FileDialogResult::SUCCESS;
		}

		gtk_widget_hide_on_delete(dialog);
		while (g_main_context_pending(nullptr)) {
			g_main_context_iteration(nullptr, FALSE);
		}
		gtk_widget_destroy(dialog);
		
		for (int i = 0; i < 5; ++i) {
			while (g_main_context_pending(nullptr)) {
				g_main_context_iteration(nullptr, FALSE);
			}
		}

		return result;
	}

	FileDialogResult FileDialog::OpenMultiple(const std::string& p_FilterList, const std::string& p_DefaultPath, std::vector<std::string>& p_OutPaths)
	{
		KTN_PROFILE_FUNCTION_LOW();

		FileDialogResult result = FileDialogResult::CANCEL;

		if (!gtk_init_check(nullptr, nullptr))
		{
			KTN_CORE_ERROR("Failed to initialize GTK!");
			return FileDialogResult::FAILED;
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Open Files",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"Cancel", GTK_RESPONSE_CANCEL,
			"Open", GTK_RESPONSE_ACCEPT,
			nullptr);

		if (!p_DefaultPath.empty())
		{
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());
		}

		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			GSList* filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
			for (GSList* iter = filenames; iter != nullptr; iter = iter->next)
			{
				p_OutPaths.push_back(static_cast<char*>(iter->data));
				g_free(iter->data);
			}
			g_slist_free(filenames);
			result = FileDialogResult::SUCCESS;
		}

		gtk_widget_destroy(dialog);

		return result;
	}

	FileDialogResult FileDialog::Save(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath)
	{
		KTN_PROFILE_FUNCTION_LOW();

		FileDialogResult result = FileDialogResult::CANCEL;

		if (!gtk_init_check(nullptr, nullptr))
		{
			KTN_CORE_ERROR("Failed to initialize GTK!");
			return FileDialogResult::FAILED;
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Save File",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			"Cancel", GTK_RESPONSE_CANCEL,
			"Save", GTK_RESPONSE_ACCEPT,
			nullptr);

		if (!p_DefaultPath.empty())
		{
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());
		}

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			p_OutPath = filename;
			g_free(filename);
			result = FileDialogResult::SUCCESS;
		}

		gtk_widget_destroy(dialog);

		return result;
	}

	FileDialogResult FileDialog::PickFolder(const std::string& p_DefaultPath, std::string& p_OutPath)
	{
		KTN_PROFILE_FUNCTION_LOW();
		
		FileDialogResult result = FileDialogResult::CANCEL;

		if (!gtk_init_check(nullptr, nullptr))
		{
			KTN_CORE_ERROR("Failed to initialize GTK!");
			return FileDialogResult::FAILED;
		}

		GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Select Folder",
			nullptr,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			"Cancel", GTK_RESPONSE_CANCEL,
			"Select", GTK_RESPONSE_ACCEPT,
			nullptr);

		if (!p_DefaultPath.empty())
		{
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());
		}

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			p_OutPath = filename;
			g_free(filename);
			result = FileDialogResult::SUCCESS;
		}

		gtk_widget_destroy(dialog);

		return result;
	}
} // namespace KTN
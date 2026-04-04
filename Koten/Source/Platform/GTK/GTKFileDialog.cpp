#include "ktnpch.h"
#include "Koten/OS/FileDialog.h"

// lib
#include <gtk/gtk.h>

// std
#include <iostream>
#include <sstream>



namespace KTN
{
    namespace
    {
        static std::vector<std::string> SplitPatterns(const std::string& p_PatternList)
        {
            KTN_PROFILE_FUNCTION_LOW();

            std::vector<std::string> result;
            std::stringstream ss(p_PatternList);
            std::string item;

            while (std::getline(ss, item, ';'))
            {
                if (!item.empty())
                    result.push_back(item);
            }

            return result;
        }

        static void ApplyFilters(GtkFileChooser* p_Dialog, const FilterList& p_Filters)
        {
            KTN_PROFILE_FUNCTION_LOW();

            for (const auto& [name, patternList] : p_Filters)
            {
                GtkFileFilter* filter = gtk_file_filter_new();
                gtk_file_filter_set_name(filter, name.c_str());

                auto patterns = SplitPatterns(patternList);

                for (const auto& pattern : patterns)
                {
                    if (pattern == "*.*")
                        gtk_file_filter_add_pattern(filter, "*");
                    else
                        gtk_file_filter_add_pattern(filter, pattern.c_str());
                }

                gtk_file_chooser_add_filter(p_Dialog, filter);
            }
        }

    } // namespace

    FileDialogResult FileDialog::Open(const FilterList& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath) 
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

        ApplyFilters(GTK_FILE_CHOOSER(dialog), p_FilterList);

        if (!p_DefaultPath.empty())
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());

        gint response      = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_ACCEPT)
        {
            char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            p_OutPath      = filename;
            g_free(filename);
            result         = FileDialogResult::SUCCESS;
        }

        while (g_main_context_pending(nullptr))
        {
            g_main_context_iteration(nullptr, FALSE);
        }

        gtk_widget_destroy(dialog);
        
        for (int i = 0; i < 5; ++i)
        {
            while (g_main_context_pending(nullptr))
            {
                g_main_context_iteration(nullptr, FALSE);
            }
        }

        return result;
    }

    FileDialogResult FileDialog::OpenMultiple(const FilterList& p_FilterList, const std::string& p_DefaultPath, std::vector<std::string>& p_OutPaths)
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

        ApplyFilters(GTK_FILE_CHOOSER(dialog), p_FilterList);

        if (!p_DefaultPath.empty())
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());

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
            result            = FileDialogResult::SUCCESS;
        }

        gtk_widget_destroy(dialog);

        return result;
    }

    FileDialogResult FileDialog::Save(const FilterList& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath)
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

        ApplyFilters(GTK_FILE_CHOOSER(dialog), p_FilterList);

        if (!p_DefaultPath.empty())
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        {
            char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            p_OutPath      = filename;
            g_free(filename);
            result         = FileDialogResult::SUCCESS;
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
            "Select a Folder",
            nullptr,
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            "Cancel", GTK_RESPONSE_CANCEL,
            "Select", GTK_RESPONSE_ACCEPT,
            nullptr);

        if (!p_DefaultPath.empty())
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), p_DefaultPath.c_str());

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        {
            char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            p_OutPath      = filename;
            g_free(filename);
            result         = FileDialogResult::SUCCESS;
        }

        gtk_widget_destroy(dialog);

        return result;
    }
} // namespace KTN
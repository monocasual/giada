#include "src/gui/dialogs/browser/nativeProjectDialog.h"
#include <FL/Fl_Native_File_Chooser.H>
#include <filesystem>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <juce_core/juce_core.h>
#include <shobjidl.h>
#include <wrl/client.h>
#include <filesystem>
#include <set>
#ifdef WITH_TESTS
#include <functional>
#endif

namespace giada::v
{
namespace
{
using Microsoft::WRL::ComPtr;
#ifdef WITH_TESTS
// Tests can arrange the dialog before Show; they still submit the real Save button.
std::function<void(IFileDialog*)> onDialogCreatedForTest;
#endif

// Windows can retain a deleted bundle's folder identity in the Save dialog.
// Refresh after deletion without accepting the dialog or losing the typed name.
class SaveFolderRefresh
{
public:
    explicit SaveFolderRefresh(IFileDialog* dialog) : dialog_(dialog), previous_(active_)
    {
        active_ = this;
        refresh_();
        timer_ = SetTimer(nullptr, 0, 250, tick_);
    }
    ~SaveFolderRefresh()
    {
        if (timer_)
            KillTimer(nullptr, timer_);
        active_ = previous_;
    }

private:
    static void CALLBACK tick_(HWND, UINT, UINT_PTR timer, DWORD)
    {
        if (active_ && active_->timer_ == timer)
            active_->refresh_();
    }

    void refresh_()
    {
        if (refreshing_)
            return;
        ComPtr<IShellItem> item;
        PWSTR text = nullptr;
        if (FAILED(dialog_->GetFolder(&item)) ||
            FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &text)))
            return;
        std::filesystem::path folder(text);
        CoTaskMemFree(text);
        std::error_code error;
        bool deleted = false;
        if (folder == folder_)
            for (const auto& project : projects_)
            {
                const bool exists = std::filesystem::exists(project, error);
                if (!error && !exists)
                    deleted = true;
                error.clear();
            }

        // If the current location itself was deleted, recover to its parent.
        while (!folder.empty() && !std::filesystem::is_directory(folder, error))
        {
            if (error && error != std::errc::no_such_file_or_directory)
                return;
            const auto parent = folder.parent_path();
            if (parent == folder)
                return;
            folder = parent;
            deleted = true;
            error.clear();
        }
        if (folder.empty())
            return;

        if (deleted)
        {
            ComPtr<IShellItem> destination;
            if (SUCCEEDED(SHCreateItemFromParsingName(folder.c_str(), nullptr,
                    IID_PPV_ARGS(destination.GetAddressOf()))))
            {
                PWSTR name = nullptr;
                dialog_->GetFileName(&name);
                refreshing_ = true;
                dialog_->SetFolder(destination.Get());
                if (name)
                {
                    dialog_->SetFileName(name);
                    CoTaskMemFree(name);
                }
                refreshing_ = false;
            }
        }
        folder_ = folder;
        projects_.clear();
        for (std::filesystem::directory_iterator it(folder, error), end;
             !error && it != end; it.increment(error))
            if (it->path().extension() == L".gprj")
                projects_.insert(it->path());
    }

    inline static thread_local SaveFolderRefresh* active_ = nullptr;
    IFileDialog* dialog_;
    SaveFolderRefresh* previous_;
    UINT_PTR timer_ = 0;
    bool refreshing_ = false;
    std::filesystem::path folder_;
    std::set<std::filesystem::path> projects_;
};
} // namespace

static NativeProjectSelection showWindowsFileDialog_(bool save, bool sample, void* owner,
    const std::string& title, const std::string& buttonLabel,
    const std::string& directory, const std::string& name,
    const std::string& nameLabel, const std::string& folderLabel)
{
    NativeProjectSelection result;
    const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    struct ComScope
    {
        bool initialized;
        ~ComScope() { if (initialized) CoUninitialize(); }
    } comScope{SUCCEEDED(initialized)};
    if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE)
        return result;

    ComPtr<IFileDialog> dialog;
    if (FAILED(CoCreateInstance(save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog,
            nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.GetAddressOf()))))
        return result;

    FILEOPENDIALOGOPTIONS options;
    if (FAILED(dialog->GetOptions(&options)))
        return result;
    options |= FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR | FOS_PATHMUSTEXIST;
    if (save)
    {
        options &= ~FOS_OVERWRITEPROMPT; // The storage layer confirms bundle replacement.
        options |= FOS_NOTESTFILECREATE; // The selected item will be a folder, not a file.
    }
    else if (!sample)
        options |= FOS_PICKFOLDERS;
    else
    {
        options &= ~FOS_PICKFOLDERS;
        options |= FOS_FILEMUSTEXIST;
    }
    if (FAILED(dialog->SetOptions(options)))
        return result;
    dialog->SetTitle(juce::String::fromUTF8(title.c_str()).toWideCharPointer());
    dialog->SetOkButtonLabel(juce::String::fromUTF8(buttonLabel.c_str()).toWideCharPointer());
    if (!sample)
        dialog->SetFileNameLabel(juce::String::fromUTF8((save ? nameLabel : folderLabel).c_str()).toWideCharPointer());

    if (!directory.empty())
    {
        ComPtr<IShellItem> initial;
        if (SUCCEEDED(SHCreateItemFromParsingName(
                juce::String::fromUTF8(directory.c_str()).toWideCharPointer(), nullptr,
                IID_PPV_ARGS(initial.GetAddressOf()))))
            dialog->SetFolder(initial.Get());
    }
    if (save)
    {
        const COMDLG_FILTERSPEC types[] = {{L"Giada project", L"*.gprj"}};
        if (FAILED(dialog->SetFileTypes(1, types)) ||
            FAILED(dialog->SetDefaultExtension(L"gprj")) ||
            FAILED(dialog->SetFileName(juce::String::fromUTF8(name.c_str()).toWideCharPointer())))
            return result;
    }
    else if (sample)
    {
        const COMDLG_FILTERSPEC types[] = {
            {L"Audio files", L"*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg"},
            {L"All files", L"*.*"}};
        if (FAILED(dialog->SetFileTypes(2, types)))
            return result;
    }

    // Browsing must never accept the dialog. Only the native Save/Open action
    // may return a selection to the storage layer.
#ifdef WITH_TESTS
    if (onDialogCreatedForTest)
        onDialogCreatedForTest(dialog.Get());
#endif
    HRESULT shown;
    if (save)
    {
        SaveFolderRefresh refresh(dialog.Get());
        shown = dialog->Show(static_cast<HWND>(owner));
    }
    else
        shown = dialog->Show(static_cast<HWND>(owner));
    if (shown == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        result.status = NativeProjectSelection::Status::CANCELLED;
        return result;
    }
    if (FAILED(shown))
        return result;

    ComPtr<IShellItem> selected;
    PWSTR path = nullptr;
    if (FAILED(dialog->GetResult(&selected)) || FAILED(selected->GetDisplayName(SIGDN_FILESYSPATH, &path)))
        return result;
    const std::string selectedPath = juce::String(path).toStdString();
    CoTaskMemFree(path);
    if (save || sample)
    {
        const juce::File project{juce::String::fromUTF8(selectedPath.c_str())};
        result.directory = project.getParentDirectory().getFullPathName().toStdString();
        result.name = (!sample && project.hasFileExtension("gprj") ? project.getFileNameWithoutExtension() : project.getFileName()).toStdString();
    }
    else
        result.directory = selectedPath;
    result.status = NativeProjectSelection::Status::SELECTED;
    return result;
}

NativeProjectSelection showNativeProjectDialog(bool save, void* owner,
    const std::string& title, const std::string& buttonLabel,
    const std::string& directory, const std::string& name,
    const std::string& nameLabel, const std::string& folderLabel)
{
    return showWindowsFileDialog_(save, false, owner, title, buttonLabel,
        directory, name, nameLabel, folderLabel);
}

NativeSampleSelection showNativeSampleDialog(void* owner, const std::string& title,
    const std::string& buttonLabel, const std::string& directory)
{
    const auto selected = showWindowsFileDialog_(false, true, owner, title,
        buttonLabel, directory, "", "", "");
    NativeSampleSelection result;
    result.status = selected.status;
    if (result.status == NativeSampleSelection::Status::SELECTED)
        result.path = juce::File(juce::String::fromUTF8(selected.directory.c_str()))
            .getChildFile(juce::String::fromUTF8(selected.name.c_str())).getFullPathName().toStdString();
    return result;
}
} // namespace giada::v
#ifdef WITH_TESTS
#include "tests/nativeProjectDialog.cpp"
#endif
#endif

// FLTK supplies Cocoa on macOS and the available desktop chooser on Linux
// (zenity/KDE/GTK, falling back to FLTK if no native backend is installed).
// Compile this adapter on Windows too, so ordinary builds check its API usage.
namespace giada::v
{
NativeProjectSelection showFltkNativeProjectDialog(bool save,
    const std::string& title, const std::string& directory, const std::string& name)
{
    NativeProjectSelection result;
    Fl_Native_File_Chooser chooser(save ? Fl_Native_File_Chooser::BROWSE_SAVE_FILE
                                       : Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    chooser.title(title.c_str());
    chooser.directory(directory.c_str());
    const std::string filename = std::filesystem::u8path(name).extension() == ".gprj" ? name : name + ".gprj";
    if (save)
    {
        chooser.filter("Giada project\t*.gprj");
        chooser.preset_file(filename.c_str());
        // The application's storage layer confirms replacement of bundle folders.
        chooser.options(Fl_Native_File_Chooser::NEW_FOLDER);
    }
    const int shown = chooser.show();
    if (shown == 1)
    {
        result.status = NativeProjectSelection::Status::CANCELLED;
        return result;
    }
    if (shown != 0 || !chooser.filename() || !*chooser.filename())
        return result;
    if (save)
    {
        const auto project = std::filesystem::u8path(chooser.filename());
        result.directory = project.parent_path().string();
        result.name = (project.extension() == ".gprj" ? project.stem() : project.filename()).string();
    }
    else
        result.directory = chooser.filename();
    result.status = NativeProjectSelection::Status::SELECTED;
    return result;
}

NativeSampleSelection showFltkNativeSampleDialog(const std::string& title,
    const std::string& directory)
{
    NativeSampleSelection result;
    Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser.title(title.c_str());
    chooser.directory(directory.c_str());
    chooser.filter("Audio files\t*.{wav,mp3,aif,aiff,flac,ogg}\nAll files\t*");
    const int shown = chooser.show();
    if (shown == 1)
        result.status = NativeSampleSelection::Status::CANCELLED;
    else if (shown == 0 && chooser.filename() && *chooser.filename())
    {
        result.path = chooser.filename();
        result.status = NativeSampleSelection::Status::SELECTED;
    }
    return result;
}

#ifndef _WIN32
NativeProjectSelection showNativeProjectDialog(bool save, void*,
    const std::string& title, const std::string&, const std::string& directory,
    const std::string& name, const std::string&, const std::string&)
{
    return showFltkNativeProjectDialog(save, title, directory, name);
}

NativeSampleSelection showNativeSampleDialog(void*, const std::string& title,
    const std::string&, const std::string& directory)
{
    return showFltkNativeSampleDialog(title, directory);
}
#endif
} // namespace giada::v

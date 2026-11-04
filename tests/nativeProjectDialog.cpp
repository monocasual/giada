// Included by nativeProjectDialog.cpp only in Windows builds with WITH_TESTS.
// Hidden Catch tests: --run-tests "[windows-gui]". No audio/MIDI or user settings.
#include "src/core/patchFactory.h"
#include "src/utils/fs.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <fstream>
#include <shlobj.h>
#include <juce_graphics/juce_graphics.h>
#include "src/gui/elems/config/tabMisc.h"
#include "src/gui/elems/basics/choice.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/fileBrowser.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/dialogs/browser/browserLoad.h"
#include "src/gui/ui.h"
#include "src/gui/drawing.h"
#include "src/core/engine.h"
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>

extern giada::v::Ui* g_ui;
extern giada::m::Engine* g_engine;

namespace native_dialog_tests
{
namespace fs = std::filesystem;
using giada::v::NativeProjectSelection;
using giada::v::showNativeProjectDialog;
using Microsoft::WRL::ComPtr;

enum class Scenario { SAVE_NEW, DELETE_REUSE, DELETE_NEW_NAME, DELETE_CURRENT, BROWSE_CANCEL, CANCEL, OPEN, OPEN_SAMPLE, CANCEL_SAMPLE };

class DialogRun
{
public:
    explicit DialogRun(Scenario scenario) : scenario_(scenario)
    {
        const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        root = fs::temp_directory_path() / ("giada-gui-test-" + std::to_string(unique));
        if (!fs::equivalent(root.parent_path(), fs::temp_directory_path()))
            throw std::runtime_error("Test directory escaped the temporary folder");
        fs::create_directory(root);
        bundle = root / "Old project.gprj";
        fs::create_directory(bundle);
        std::ofstream(bundle / "Old project.gptc") << "{}";
        std::ofstream(root / "Preview sample.wav") << "Temporary file-picker fixture";
        initialName_ = scenario == Scenario::DELETE_REUSE ? L"Old project.gprj" : L"New melody.v2.gprj";
        giada::v::onDialogCreatedForTest = [this](IFileDialog* dialog)
        {
            dialog_ = dialog;
            if (scenario_ == Scenario::OPEN_SAMPLE)
                dialog_->SetFileName(L"Preview sample.wav");
            started_ = GetTickCount64();
            active_ = this;
            timer_ = SetTimer(nullptr, 0, 100, tick_);
            if (!timer_)
                throw std::runtime_error("Cannot create native dialog test timer");
        };
    }

    ~DialogRun()
    {
        if (timer_)
            KillTimer(nullptr, timer_);
        active_ = nullptr;
        giada::v::onDialogCreatedForTest = {};
        // Only delete the unique temporary root that this fixture created.
        std::error_code locationError;
        if (fs::equivalent(root.parent_path(), fs::temp_directory_path(), locationError) &&
            root.filename().wstring().starts_with(L"giada-gui-test-"))
        {
            std::error_code ignored;
            fs::remove_all(root, ignored);
        }
    }

    NativeProjectSelection run()
    {
        const bool save = scenario_ != Scenario::OPEN;
        const auto startFolder = scenario_ == Scenario::DELETE_CURRENT || scenario_ == Scenario::OPEN ? bundle : root;
        return showNativeProjectDialog(save, nullptr, "Giada native dialog automated test",
            save ? "Save test project" : "Open test project", utf8_(startFolder),
            juce::String(initialName_.c_str()).toStdString(), "Project name", "Project folder");
    }

    giada::v::NativeSampleSelection runSample()
    {
        return giada::v::showNativeSampleDialog(nullptr, "Giada native dialog automated test",
            "Open test sample", utf8_(root));
    }

    fs::path root, bundle;
    std::string failure;
    bool submitted = false;
    bool preservedName = false;
    bool stayedOpen = false;

    void snapshot(HWND window, const char* phase, bool native = true, bool sidebar = true)
    {
        capture_(window, phase, native, sidebar);
    }

    void snapshotWidget(Fl_Widget& widget, const char* phase, bool hidePath)
    {
        const auto output = juce::SystemStats::getEnvironmentVariable("GIADA_GUI_TEST_ARTIFACTS", "");
        if (output.isEmpty()) return;
        // FLTK double-buffered widgets do not reliably implement WM_PRINTCLIENT.
        // Render the actual widget tree to FLTK's image surface instead.
        std::unique_ptr<Fl_RGB_Image> rendered;
        {
            Fl_Image_Surface surface(widget.w(), widget.h());
            Fl_Surface_Device::push_current(&surface);
            fl_font(FL_HELVETICA, giada::G_GUI_FONT_SIZE_BASE);
            surface.draw(&widget);
            rendered.reset(surface.image());
            Fl_Surface_Device::pop_current();
        }
        juce::Image image(juce::Image::RGB, rendered->w(), rendered->h(), true);
        const auto* pixels = reinterpret_cast<const unsigned char*>(rendered->data()[0]);
        const int stride = rendered->ld() ? rendered->ld() : rendered->w() * rendered->d();
        for (int y = 0; y < rendered->h(); ++y)
            for (int x = 0; x < rendered->w(); ++x)
            {
                const auto* pixel = pixels + y * stride + x * rendered->d();
                image.setPixelAt(x, y, juce::Colour(pixel[0], pixel[1], pixel[2]));
            }
        if (hidePath)
        {
            juce::Graphics graphics(image);
            graphics.setColour(juce::Colour(32, 32, 32));
            graphics.fillRect(0, 0, image.getWidth(), 65);
        }
        const auto file = juce::File(output).getChildFile(juce::String("0-") + phase + "-redacted.png");
        file.getParentDirectory().createDirectory();
        juce::FileOutputStream stream(file);
        if (!stream.openedOk() || !stream.setPosition(0) || stream.truncate().failed() ||
            !juce::PNGImageFormat().writeImageToStream(image, stream))
            failure = "Could not save redacted widget screenshot";
    }

private:
    static std::string utf8_(const fs::path& path)
    {
        return juce::String(path.c_str()).toStdString();
    }

    static void CALLBACK tick_(HWND, UINT, UINT_PTR timer, DWORD)
    {
        if (!active_ || active_->timer_ != timer || active_->inTick_)
            return;
        active_->inTick_ = true;
        try { active_->step_(); }
        catch (const std::exception& error) { active_->fail_(error.what()); }
        active_->inTick_ = false;
    }

    void fail_(const std::string& message)
    {
        failure = message;
        // Close only dialogs belonging to this test thread (including any native
        // error prompt). Never target another Giada instance or user's windows.
        EnumThreadWindows(GetCurrentThreadId(), [](HWND window, LPARAM) -> BOOL
        {
            PostMessageW(window, WM_CLOSE, 0, 0);
            return TRUE;
        }, 0);
        dialog_->Close(HRESULT_FROM_WIN32(ERROR_CANCELLED));
    }

    void click_(HWND window, int id)
    {
        const HWND button = GetDlgItem(window, id);
        if (!button || !IsWindowEnabled(button))
            throw std::runtime_error("Native dialog Save/Cancel button missing or disabled");
        if (id == IDOK)
            submitted = true;
        // Exercise the real native button and its validation; Close(S_OK) would
        // bypass that validation and hide the regression we are testing.
        PostMessageW(button, BM_CLICK, 0, 0);
    }

    void capture_(HWND window, const char* phase, bool native = true, bool sidebar = true)
    {
        const auto output = juce::SystemStats::getEnvironmentVariable("GIADA_GUI_TEST_ARTIFACTS", "");
        if (output.isEmpty())
            return;
        RECT bounds{};
        GetClientRect(window, &bounds);
        const int width = bounds.right, height = bounds.bottom;
        HDC source = GetDC(window);
        HDC target = CreateCompatibleDC(source);
        BITMAPINFO format{};
        format.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        format.bmiHeader.biWidth = width;
        format.bmiHeader.biHeight = -height;
        format.bmiHeader.biPlanes = 1;
        format.bmiHeader.biBitCount = 32;
        format.bmiHeader.biCompression = BI_RGB;
        void* pixels = nullptr;
        HBITMAP bitmap = CreateDIBSection(target, &format, DIB_RGB_COLORS, &pixels, nullptr, 0);
        HGDIOBJ previous = bitmap ? SelectObject(target, bitmap) : nullptr;
        const BOOL captured = bitmap && PrintWindow(window, target, 3);
        if (captured)
        {
            juce::Image image(juce::Image::RGB, width, height, true);
            const auto* bytes = static_cast<const unsigned char*>(pixels);
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < width; ++x)
                {
                    const auto* pixel = bytes + (y * width + x) * 4;
                    image.setPixelAt(x, y, juce::Colour(pixel[2], pixel[1], pixel[0]));
                }
            // Redact before saving: the shell sidebar can show the real Windows
            // account even though all project data is from a temporary fixture.
            // Opaque masks prevent recovery; raw screenshots are never written.
            {
                juce::Graphics graphics(image);
                graphics.setColour(juce::Colour(32, 32, 32));
                if (native)
                {
                    graphics.fillRect(0, 0, width, (std::min)(sidebar ? 150 : 65, height));
                    struct RedactionBounds { HWND window; int width, height, footer; } layout{window, width, height, height};
                    EnumChildWindows(window, [](HWND child, LPARAM parameter) -> BOOL
                    {
                        auto& layout = *reinterpret_cast<RedactionBounds*>(parameter);
                        wchar_t type[64]{};
                        GetClassNameW(child, type, 64);
                        RECT field{};
                        if ((wcscmp(type, L"Edit") == 0 || wcscmp(type, L"ComboBox") == 0) &&
                            GetWindowRect(child, &field))
                        {
                            MapWindowPoints(nullptr, layout.window, reinterpret_cast<POINT*>(&field), 2);
                            if (field.top > layout.height / 2 && field.right - field.left > layout.width / 2)
                                layout.footer = (std::min)(layout.footer, static_cast<int>(field.top) - 4);
                        }
                        return TRUE;
                    }, reinterpret_cast<LPARAM>(&layout));
                    if (sidebar)
                        graphics.fillRect(0, 0, width * 35 / 100, layout.footer);
                }
            }
            const auto file = juce::File(output).getChildFile(
                juce::String(static_cast<int>(scenario_)) + "-" + phase + "-redacted.png");
            file.getParentDirectory().createDirectory();
            juce::FileOutputStream stream(file);
            if (!stream.openedOk() || !stream.setPosition(0) || stream.truncate().failed() ||
                !juce::PNGImageFormat().writeImageToStream(image, stream))
                failure = "Could not save redacted GUI screenshot";
        }
        if (previous) SelectObject(target, previous);
        if (bitmap) DeleteObject(bitmap);
        DeleteDC(target);
        ReleaseDC(window, source);
    }

    void step_()
    {
        if (GetTickCount64() - started_ > 10000)
        {
            fail_("Native dialog did not finish within 10 seconds (possibly a blocking error prompt)");
            return;
        }
        ComPtr<IOleWindow> ole;
        HWND window = nullptr;
        if (FAILED(dialog_->QueryInterface(IID_PPV_ARGS(ole.GetAddressOf()))) ||
            FAILED(ole->GetWindow(&window)) || !IsWindowVisible(window))
            return;
        if (stage_ == 0)
        {
            if (GetTickCount64() - started_ < 500)
                return;
            capture_(window, "before");
            if (scenario_ == Scenario::CANCEL || scenario_ == Scenario::OPEN ||
                scenario_ == Scenario::OPEN_SAMPLE || scenario_ == Scenario::CANCEL_SAMPLE)
            {
                stayedOpen = true;
                stage_ = 2;
                click_(window, scenario_ == Scenario::CANCEL || scenario_ == Scenario::CANCEL_SAMPLE ? IDCANCEL : IDOK);
                return;
            }
            if (scenario_ == Scenario::BROWSE_CANCEL)
            {
                ComPtr<IShellItem> folder;
                if (FAILED(SHCreateItemFromParsingName(bundle.c_str(), nullptr,
                        IID_PPV_ARGS(folder.GetAddressOf()))) || FAILED(dialog_->SetFolder(folder.Get())))
                    throw std::runtime_error("Cannot browse into the temporary project");
            }
            else if (scenario_ != Scenario::SAVE_NEW)
            {
                if (scenario_ == Scenario::DELETE_REUSE)
                {
                    // Seed the shell's selected-folder state before deletion,
                    // rather than testing only an unselected filesystem change.
                    ComPtr<IServiceProvider> services;
                    ComPtr<IFolderView2> view;
                    ComPtr<IShellFolder> folder;
                    if (FAILED(dialog_->QueryInterface(IID_PPV_ARGS(services.GetAddressOf()))) ||
                        FAILED(services->QueryService(SID_SFolderView, IID_PPV_ARGS(view.GetAddressOf()))) ||
                        FAILED(view->GetFolder(IID_PPV_ARGS(folder.GetAddressOf()))))
                        throw std::runtime_error("Cannot access native dialog folder view");
                    PIDLIST_RELATIVE child = nullptr;
                    auto filename = bundle.filename().wstring();
                    if (FAILED(folder->ParseDisplayName(nullptr, nullptr, filename.data(), nullptr, &child, nullptr)))
                        throw std::runtime_error("Cannot locate fixture project in native dialog");
                    PCUITEMID_CHILD children[] = {child};
                    const HRESULT selected = view->SelectAndPositionItems(1, children, nullptr,
                        SVSI_SELECT | SVSI_FOCUSED | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS);
                    CoTaskMemFree(child);
                    if (FAILED(selected))
                        throw std::runtime_error("Cannot select fixture project in native dialog");
                }
                // Delete exactly our own fixture bundle while the actual dialog
                // is open. Notify the shell as Explorer's Delete command does.
                if (!fs::remove(bundle / "Old project.gptc") || !fs::remove(bundle))
                    throw std::runtime_error("Cannot delete temporary project");
                SHChangeNotify(SHCNE_RMDIR, SHCNF_PATHW | SHCNF_FLUSH, bundle.c_str(), nullptr);
            }
            changed_ = GetTickCount64();
            stage_ = 1;
        }
        else if (stage_ == 1 && GetTickCount64() - changed_ > 1000)
        {
            capture_(window, "after");
            stayedOpen = true;
            PWSTR name = nullptr;
            if (SUCCEEDED(dialog_->GetFileName(&name)))
            {
                preservedName = name && initialName_ == name;
                CoTaskMemFree(name);
            }
            stage_ = 2;
            click_(window, scenario_ == Scenario::BROWSE_CANCEL ? IDCANCEL : IDOK);
        }
    }

    inline static thread_local DialogRun* active_ = nullptr;
    Scenario scenario_;
    IFileDialog* dialog_ = nullptr;
    UINT_PTR timer_ = 0;
    ULONGLONG started_ = 0, changed_ = 0;
    int stage_ = 0;
    bool inTick_ = false;
    std::wstring initialName_;
};
} // namespace native_dialog_tests

namespace native_dialog_tests
{
struct TestUi
{
    std::unique_ptr<giada::v::Ui> uiStorage = std::make_unique<giada::v::Ui>();
    std::unique_ptr<giada::m::Engine> engineStorage = std::make_unique<giada::m::Engine>();
    giada::v::Ui& ui = *uiStorage;
    giada::v::Ui* previousUi = g_ui;
    giada::m::Engine* previousEngine = g_engine;
    TestUi()
    {
        Fl::lock(); // Match Ui::init before pumping the Windows event loop.
        g_ui = &ui;
        g_engine = engineStorage.get();
        Fl::background(25, 25, 25);
        Fl::foreground(200, 200, 200);
        Fl::set_boxtype(G_CUSTOM_BORDER_BOX, g_customBorderBox, 1, 1, 2, 2);
        Fl::set_boxtype(G_CUSTOM_UP_BOX, g_customUpBox, 1, 1, 2, 2);
        Fl::set_boxtype(G_CUSTOM_DOWN_BOX, g_customDownBox, 1, 1, 2, 2);
        Fl::set_boxtype(FL_BORDER_BOX, G_CUSTOM_BORDER_BOX);
        Fl::set_boxtype(FL_UP_BOX, G_CUSTOM_UP_BOX);
        Fl::set_boxtype(FL_DOWN_BOX, G_CUSTOM_DOWN_BOX);
        ui.model.browserPosition = 0;
        ui.model.browserLastValue = 0;
    }
    ~TestUi() { g_ui = previousUi; g_engine = previousEngine; }
};

giada::v::geChoice* findBrowserChoice(Fl_Group& group)
{
    for (int i = 0; i < group.children(); ++i)
    {
        if (auto* choice = dynamic_cast<giada::v::geChoice*>(group.child(i));
            choice && choice->hasItem("Giada browser (experimental)"))
            return choice;
        if (auto* child = dynamic_cast<Fl_Group*>(group.child(i)))
            if (auto* found = findBrowserChoice(*child))
                return found;
    }
    return nullptr;
}

class PreviewBrowser : public giada::v::gdBrowserLoad
{
public:
    PreviewBrowser(const giada::v::Model& model, const std::string& directory, bool audio)
        : gdBrowserLoad("Giada browser test", directory, [](void*) {}, {}, model, audio) {}
    bool previewVisible() const { return preview->visible(); }
    int previewWidth() const { return preview->w(); }
    Fl_Input* pathInput() const { return dynamic_cast<Fl_Input*>(where->child(0)); }
    std::string listedPath() const { return browser->getCurrentDir(); }
    bool lists(const std::string& name) const
    {
        for (int row = 1; row <= browser->size(); ++row)
            if (browser->text(row) && name == browser->text(row)) return true;
        return false;
    }
};
}

TEST_CASE("Sample browser setting switches immediately and survives configuration round trip", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    TestUi state;
    DialogRun evidence(Scenario::SAVE_NEW);
    Fl_Double_Window window(640, 320, "Giada settings test");
    auto* tab = new giada::v::geTabMisc({0, 0, 640, 320});
    window.end();
    window.show();
    window.make_current();
    fl_font(FL_HELVETICA, giada::G_GUI_FONT_SIZE_BASE);
    Fl::check();
    auto* choice = findBrowserChoice(*tab);
    REQUIRE(choice);
    CHECK_FALSE(giada::m::Conf{}.useNativeSampleDialogs);
    CHECK(choice->getSelectedId() == 0);
    CHECK(choice->getSelectedLabel() == "Giada browser (experimental)");
    evidence.snapshotWidget(window, "settings-giada-default", false);
    choice->showItem(1);
    choice->onChange(1);
    CHECK(state.ui.model.useNativeSampleDialogs);
    giada::m::Conf saved;
    state.ui.model.store(saved);
    giada::v::Model reopened;
    reopened.load(saved);
    CHECK(reopened.useNativeSampleDialogs);
    window.redraw();
    Fl::check();
    evidence.snapshotWidget(window, "settings-os-native", false);
    choice->showItem(0);
    choice->onChange(0);
    CHECK_FALSE(state.ui.model.useNativeSampleDialogs);
    window.hide();
}

TEST_CASE("Audio browser has an optional visible preview control", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    TestUi state;
    DialogRun evidence(Scenario::SAVE_NEW);
    const auto directory = juce::String(evidence.root.c_str()).toStdString();
    {
        PreviewBrowser browser(state.ui.model, directory, false);
        browser.make_current();
        fl_font(FL_HELVETICA, giada::G_GUI_FONT_SIZE_BASE);
        CHECK_FALSE(browser.previewVisible());
        Fl::check();
        evidence.snapshotWidget(browser, "browser-before-preview", true);
        browser.hide();
    }
    {
        PreviewBrowser browser(state.ui.model, directory, true);
        browser.make_current();
        fl_font(FL_HELVETICA, giada::G_GUI_FONT_SIZE_BASE);
        CHECK(browser.previewVisible());
        CHECK(browser.previewWidth() > 0);
        Fl::check();
        evidence.snapshotWidget(browser, "browser-after-preview", true);
        browser.hide();
    }
}

TEST_CASE("Browser path field accepts a pasted quoted folder and opens it on Enter", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    TestUi state;
    DialogRun evidence(Scenario::SAVE_NEW);
    const auto destination = evidence.root / "Preview sounds with spaces";
    REQUIRE(fs::create_directory(destination));
    std::ofstream(destination / "Kick.wav") << "Temporary browser fixture";
    PreviewBrowser browser(state.ui.model, juce::String(evidence.root.c_str()).toStdString(), true);
    browser.make_current();
    fl_font(FL_HELVETICA, giada::G_GUI_FONT_SIZE_BASE);
    Fl::check();
    auto* input = browser.pathInput();
    REQUIRE(input);
    CHECK_FALSE(input->readonly());
    CHECK(input->when() == FL_WHEN_ENTER_KEY_ALWAYS);
    evidence.snapshotWidget(browser, "path-before-enter", true);
    // Exercise insertion and the input's real Enter callback without changing
    // the user's system clipboard during an automated test.
    const auto pasted = std::string("\"") + juce::String(destination.c_str()).toStdString() + "\"";
    input->replace(0, input->size(), pasted.c_str());
    CHECK(fs::equivalent(fs::path(browser.listedPath()), evidence.root));
    CHECK(fs::equivalent(fs::path(browser.getCurrentPath()), evidence.root));
    input->do_callback();
    CHECK(fs::equivalent(fs::path(browser.listedPath()), destination));
    CHECK(browser.lists("Kick.wav"));
    input->do_callback(); // Enter also works without another text edit.
    CHECK(fs::equivalent(fs::path(browser.listedPath()), destination));
    Fl::check();
    evidence.snapshotWidget(browser, "path-after-enter", true);
    browser.hide();
}

TEST_CASE("Native Save accepts a new project through its real Save button", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    DialogRun dialog(Scenario::SAVE_NEW);
    const auto selection = dialog.run();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    REQUIRE(dialog.submitted);
    REQUIRE(selection.status == NativeProjectSelection::Status::SELECTED);
    CHECK(fs::path(selection.directory) == dialog.root);
    CHECK(selection.name == "New melody.v2");
    CHECK(dialog.preservedName);
}

TEST_CASE("Native Save works after deleting projects while it is open", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    const auto scenario = GENERATE(Scenario::DELETE_REUSE, Scenario::DELETE_NEW_NAME, Scenario::DELETE_CURRENT);
    DialogRun dialog(scenario);
    const auto selection = dialog.run();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    REQUIRE(dialog.stayedOpen);
    REQUIRE(dialog.submitted);
    REQUIRE(selection.status == NativeProjectSelection::Status::SELECTED);
    REQUIRE(fs::path(selection.directory) == dialog.root);
    CHECK(dialog.preservedName);
    const auto saved = fs::path(giada::u::fs::getProjectSavePath(selection.directory, selection.name));
    REQUIRE(fs::create_directory(saved));
    giada::m::Patch patch;
    patch.name = selection.name;
    const auto document = saved / (selection.name + ".gptc");
    REQUIRE(giada::m::patchFactory::serialize(patch, document.string()));
    const auto reopened = giada::m::patchFactory::deserialize(document.string());
    CHECK(reopened.status == giada::G_FILE_OK);
    CHECK(reopened.name == patch.name);
}

TEST_CASE("Native Save does not accept merely browsing an existing project", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    DialogRun dialog(Scenario::BROWSE_CANCEL);
    const auto selection = dialog.run();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    CHECK(dialog.stayedOpen);
    CHECK_FALSE(dialog.submitted);
    CHECK(selection.status == NativeProjectSelection::Status::CANCELLED);
    CHECK(fs::exists(dialog.bundle / "Old project.gptc"));
}

TEST_CASE("Cancelling Native Save returns no project", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    DialogRun dialog(Scenario::CANCEL);
    const auto selection = dialog.run();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    CHECK(selection.status == NativeProjectSelection::Status::CANCELLED);
    CHECK(selection.directory.empty());
    CHECK(selection.name.empty());
}

TEST_CASE("Native Open returns a project bundle", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    DialogRun dialog(Scenario::OPEN);
    const auto selection = dialog.run();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    REQUIRE(dialog.submitted);
    REQUIRE(selection.status == NativeProjectSelection::Status::SELECTED);
    CHECK(fs::path(selection.directory) == dialog.bundle);
}

TEST_CASE("Native sample picker selects an audio file or cancels without a path", "[.][windows-gui]")
{
    using namespace native_dialog_tests;
    const auto scenario = GENERATE(Scenario::OPEN_SAMPLE, Scenario::CANCEL_SAMPLE);
    DialogRun dialog(scenario);
    const auto selection = dialog.runSample();
    INFO(dialog.failure);
    REQUIRE(dialog.failure.empty());
    REQUIRE(dialog.stayedOpen);
    if (scenario == Scenario::OPEN_SAMPLE)
    {
        REQUIRE(dialog.submitted);
        REQUIRE(selection.status == giada::v::NativeSampleSelection::Status::SELECTED);
        CHECK(fs::path(selection.path) == dialog.root / "Preview sample.wav");
    }
    else
    {
        CHECK_FALSE(dialog.submitted);
        CHECK(selection.status == giada::v::NativeSampleSelection::Status::CANCELLED);
        CHECK(selection.path.empty());
    }
}

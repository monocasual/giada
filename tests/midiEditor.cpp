#include "src/gui/dialogs/actionEditor/baseActionEditor.h"
#include "src/gui/elems/actionEditor/pianoRoll.h"
#include "src/gui/elems/actionEditor/pianoItem.h"
#include "src/gui/ui.h"
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#include <catch2/catch_test_macros.hpp>

extern giada::v::Ui* g_ui;

namespace
{
class MidiEditorHarness : public giada::v::gdBaseActionEditor
{
public:
    explicit MidiEditorHarness(const giada::v::Model& model) : gdBaseActionEditor({}, model)
    {
        fullWidth = loopWidth = 800;
        end();
    }
    void draw() override {} // The fixture supplies actions without starting an engine.
};
}

TEST_CASE("MIDI piano roll renders orphan notes and ignores controller events", "[.][windows-gui]")
{
    using namespace giada;
    auto ui = std::make_unique<v::Ui>();
    struct RestoreUi { v::Ui* previous = g_ui; ~RestoreUi() { g_ui = previous; } } restore;
    g_ui = ui.get();
    Fl::lock();
    Fl_Double_Window display(200, 100, "MIDI editor regression test");
    display.end();
    display.show();
    display.make_current();
    fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
    MidiEditorHarness editor(ui->model);
    editor.begin();
    auto* roll = new v::gePianoRoll(0, 0, &editor);
    roll->end();
    editor.end();
    m::Action orphan{.id = ID{1}, .tick = Tick{0},
        .event = m::MidiEvent::makeFrom3Bytes(0x90, 60, 100)};
    m::Action pedal{.id = ID{2}, .tick = Tick{120},
        .event = m::MidiEvent::makeFrom3Bytes(0xb0, 64, 127)};
    m::Action release{.id = ID{3}, .tick = Tick{240},
        .event = m::MidiEvent::makeFrom3Bytes(0x80, 60, 0)};
    c::actionEditor::Data data;
    data.ticksInLoop = data.ticksInSeq = Tick{3840};
    data.ticksInBeat = Tick{960};
    data.ticksInBar = Tick{3840};
    data.actions = {&orphan, &pedal, &release};
    roll->rebuild(data);
    REQUIRE(roll->children() == 1);
    auto* item = dynamic_cast<v::gePianoItem*>(roll->child(0));
    REQUIRE(item);
    CHECK(item->a2 == nullptr);
    CHECK_FALSE(item->isResizable());
    CHECK(item->w() == v::geBaseAction::MIN_WIDTH);
    Fl_Image_Surface surface(800, roll->h());
    Fl_Surface_Device::push_current(&surface);
    roll->draw();
    Fl_Surface_Device::pop_current();
    // Matched notes and notes crossing the loop boundary retain their behavior.
    v::gePianoItem paired(0, 0, 50, 20, orphan, &release);
    CHECK(paired.isResizable());
    m::Action wrapped = release;
    wrapped.tick = Tick{0};
    m::Action late = orphan;
    late.tick = Tick{100};
    v::gePianoItem ring(0, 0, 50, 20, late, &wrapped);
    CHECK_FALSE(ring.isResizable());
    data.actions.clear();
    roll->rebuild(data);
    CHECK(roll->children() == 0);
}

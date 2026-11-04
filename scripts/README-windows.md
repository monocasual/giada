# Windows development

Prerequisites:

- Visual Studio 2022 with the Desktop development with C++ workload and a Windows SDK.
  The MSVC standard library must support C++23 `std::views::enumerate`.
- CMake 3.29 or newer.
- Git and a bootstrapped vcpkg checkout.

From the repository root, build with VST3 support, run the tests, and package a
portable copy:

```powershell
./scripts/build-windows.ps1 -VcpkgRoot C:/dev/vcpkg -OutputDirectory C:/dev/giada-portable
```

If CMake is not on PATH, supply its executable with `-CMake`. Dependency versions
come from `vcpkg.json`. The build uses the upstream Visual Studio 2022 generator
and an isolated manifest installation under `build/vcpkg_installed`.

For subsequent builds, add `-SkipConfigure`. Use `-Configuration Debug` for a
debug build or `-Launch` to open the resulting app after its tests pass. All
builds enable the existing test suite and VST3 hosting. The portable copy includes
transitive codec DLLs from vcpkg, including `libmp3lame.dll`.

If Visual Studio's installed compiler is older, `-ToolsetDirectory` can point to
a newer complete MSVC toolset directory (containing `include`, `lib`, and
`bin/Hostx64/x64`). The helper supplies its compiler, linker, headers, and
libraries to MSBuild. This local setup uses MSVC 14.44 alongside the installed
Visual Studio 2022 build host. Dependencies built with older compatible v143
tools can be reused; see Microsoft's [binary compatibility guidance](https://learn.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017).

The initial configuration downloads and compiles dependencies. Later builds reuse
them. Close the development app before rebuilding or packaging over its running
executable.

## Project and sample browsers

Project Open/Save is routed by `src/glue/layout.cpp` through the native adapter
in `src/gui/dialogs/browser/nativeProjectDialog.cpp`. The custom browser in
`browserBase.cpp` is available for sample selection, with an editable location
field. Storage callbacks are in `src/glue/storage.cpp`; actual project
serialization is in `src/core/api/storageApi.cpp`.

A project is a `.gprj` directory containing a `.gptc` document and assets, rather
than one standalone project file. On Windows, Open Project and Save Project now
use native Windows dialogs. Save uses the standard Save As dialog: choose a
destination folder and enter a name in the full-width project-name field to
create a `.gprj` bundle. The Save dialog watches for deleted project folders and
refreshes its location without accepting the dialog or losing the typed name.
If the current folder is deleted, it returns to the nearest existing parent.
The storage layer confirms replacement of existing bundles. Open selects the
`.gprj` folder itself. macOS and Linux use FLTK's native chooser adapter, with
FLTK's standard fallback where a native desktop chooser is unavailable.

Open Project and Save Project always use the native dialogs. Settings > Misc >
Sample browser switches sample selection between Giada browser (experimental,
default) and OS native. Explicit sample-browser preferences are saved on normal
shutdown. The old project-browser preference does not control sample selection.
Giada's sample browser offers Play preview / Stop preview. Changing selection or
directory, loading a sample, and closing the browser all stop preview playback.
Preview audio uses the existing output at half volume and is never added to the
project's wave list. Project dialogs do not offer audio previews.

Giada's sample browser is the default so audio preview is available
while choosing samples, and remains marked experimental while its usability is
improved. The maintainer is welcome to choose either sample-browser default.
This choice does not change the native project-saving workflow.
In Giada's sample browser, the location field
accepts typed or pasted folder paths: press Enter to navigate. Quoted paths from
Explorer's Copy as path are accepted. Invalid paths leave the current folder
unchanged and display an error.

Regression tests cover names and paths containing spaces and dots, selecting an
existing bundle without nesting projects, and serializing/reopening project data.
The build helper runs native Windows GUI tests after the unit tests. The GUI
suite opens actual project and sample dialogs and activates their Save/Open/Cancel buttons, including
deleting temporary projects and saving again, reusing a deleted name, recovering
from a deleted current folder, and browsing without accepting. No audio hardware,
user projects, or user configuration is used. The deletion is performed on test
fixtures while the dialog is open and the shell is notified; Explorer's Delete
keyboard shortcut and its confirmation prompt are not themselves automated.

Run only the GUI suite with:

```powershell
./scripts/test-windows-gui.ps1 -Executable ./build/Release/giada.exe
```

The runner enforces a timeout and saves JUnit XML, logs, and redacted before/after
dialog screenshots in `build/gui-test-results`. Personal shell navigation and
the address bar are masked before a screenshot is saved; unredacted captures are
never written. Review images before sharing. Windows CI runs this suite and
uploads the results as an artifact. A local build may explicitly use
`-SkipGuiTests` when only the non-GUI suite is wanted. Real audio-device playback
and project reopening with third-party synth settings still need listening and
integration checks.

Validation scope:

- Windows: full application build, unit suite, native dialog and FLTK control GUI
  tests, including deletion while browsing, cancellation, preference switching,
  and preview-control visibility. Audio decoding and rendering are tested without
  hardware; actual speaker playback remains a listening check.
- Linux: limited project-dialog adapter smoke tests on Ubuntu 24.04 in Docker's Linux VM under
  Xvfb/GTK, compiled against the repository's FLTK. Save, Unicode/dotted names,
  deleting/reusing a bundle name, Open, and Cancel pass. This is **not a full
  Linux application build or end-to-end audio test**. The new native sample-picker
  adapter has not been exercised on Linux.
- macOS: **not built or tested**. The native adapter still needs platform testing.

Do not describe the Windows-only regression suite as Linux/macOS validation.

Negative control: disabling the Save dialog's deletion-refresh timer makes the
deletion GUI test fail (a blocked native prompt and a deleted current location).
Restoring the timer passes the suite. This confirms the GUI checks exercise the
regression rather than only the path helper.

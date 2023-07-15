#!/usr/bin/env python3


# -- MAKE-SRC.PY ---------------------------------------------------------------
# ------------------------------------------------------------------------------


import sys
import os
import subprocess
import tempfile
import shutil
import glob


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
SCRIPTS_DIR = f"{CURRENT_DIR}/../../scripts"
TEMP_DIR = tempfile.gettempdir()
OUTPUT_DIR = "dist"


def check_input():
    if len(sys.argv) == 1:
        print("[MAKESRC] Usage: make-src.py [package-version].")
        sys.exit(1)


def exec(command):
    if subprocess.call(command) != 0:
        sys.exit(1)


def remove(pattern):
    for file in glob.glob(pattern):
        print(f"Remove {file}")
        if os.path.isfile(file):
            os.remove(file)
        elif os.path.isdir(file):
            shutil.rmtree(file)
        else:
            raise Error()


def log(msg):
    print(msg, flush=True)


# ------------------------------------------------------------------------------


check_input()

VERSION = sys.argv[1]

log("Invoke tarball script...")

exec([f"{SCRIPTS_DIR}/create_source_tarball.sh", "-v", f"{VERSION}"])

log(f"Untar the result to {TEMP_DIR}...")

exec(["tar", "zxf", f"{SCRIPTS_DIR}/giada-{VERSION}-src.tar.gz", "-C", f"{TEMP_DIR}"])

log("Remove the tar file...")

remove(f"{SCRIPTS_DIR}/giada-{VERSION}-src.tar.gz")

log("Remove useless stuff...")

remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/juce/docs")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/juce/examples")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/juce/extras/[!Build,CMakeLists.txt]*")
remove(
    f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/juce/extras/BinaryBuilder"
)  # Not deleted by command above for some obscure reason
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/juce/.github")

remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/cmake")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/contrib")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/doc")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/tests")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/ChangeLog")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/CMakeLists.txt")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/configure.ac")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/install.txt")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/LICENSE")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/Makefile.am")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/README.md")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/rtaudio.pc.in")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/rtaudio_c.cpp")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/rtaudio/autogen.sh/rtaudio_c.h")

remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/vst3sdk/doc")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/vst3sdk/public.sdk/samples")
remove(f"{TEMP_DIR}/giada-{VERSION}-src/src/deps/vst3sdk/vstgui4")

log("Create output directory...")

os.makedirs(OUTPUT_DIR)

log("Re-create tar.gz archive...")

exec(
    [
        "tar",
        "-zcvf",
        f"{OUTPUT_DIR}/giada-{VERSION}-src.tar.gz",
        "-C",
        f"{TEMP_DIR}/",
        f"giada-{VERSION}-src/",
    ]
)

log("Done.")

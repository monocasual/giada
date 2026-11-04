#include "src/utils/fs.h"
#include <juce_core/juce_core.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Startup project arguments accept bundles and contained patch files")
{
	using giada::u::fs::getProjectOpenPath;
	const juce::File bundle = juce::File::getCurrentWorkingDirectory().getChildFile("Test music/My melody.gprj");
	CHECK(getProjectOpenPath(bundle.getFullPathName().toStdString()) == bundle.getFullPathName().toStdString());
	CHECK(getProjectOpenPath(bundle.getChildFile("My melody.gptc").getFullPathName().toStdString()) == bundle.getFullPathName().toStdString());
	CHECK(getProjectOpenPath(bundle.getChildFile("My melody.GPTC").getFullPathName().toStdString()) == bundle.getFullPathName().toStdString());
	CHECK(getProjectOpenPath("").empty());
	CHECK(getProjectOpenPath("--unknown-option").empty());
	CHECK(getProjectOpenPath("Kick.wav").empty());
}

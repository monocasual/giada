#include "../src/core/channels/midiLighter.h"
#include "mocks/kernelMidiMock.h"
#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("MidiMapper")
{
	using namespace giada;

	m::KernelMidiMock                kernelMidi;
	m::MidiMapper<m::KernelMidiMock> midiMapper(kernelMidi);
	m::MidiLighter                   midiLighter(midiMapper);

	SECTION("Test initialization")
	{
		REQUIRE(midiLighter.enabled == false);
	}
}

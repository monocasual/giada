//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstprocesscontext.h
// Created by  : Steinberg, 10/2005
// Description : VST Processing Context Interfaces
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2015, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// This Software Development Kit may not be distributed in parts or its entirety
// without prior written agreement by Steinberg Media Technologies GmbH.
// This SDK must not be used to re-engineer or manipulate any technology used
// in any Steinberg or Third-party application or software module,
// unless permitted by law.
// Neither the name of the Steinberg Media Technologies nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SDK IS PROVIDED BY STEINBERG MEDIA TECHNOLOGIES GMBH "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL STEINBERG MEDIA TECHNOLOGIES GMBH BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "vsttypes.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
/** Frame Rate */
//------------------------------------------------------------------------
struct FrameRate
{
//------------------------------------------------------------------------
	enum FrameRateFlags
	{
		kPullDownRate = 1 << 0, ///< for ex. HDTV: 23.976 fps with 24 as frame rate
		kDropRate     = 1 << 1	///< for ex. 29.97 fps drop with 30 as frame rate
	};
//------------------------------------------------------------------------
	uint32 framesPerSecond;		///< frame rate
	uint32 flags;				///< flags #FrameRateFlags
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Description of a chord.
A chord is described with a key note, a root note and the
\copydoc chordMask
\see ProcessContext*/
//------------------------------------------------------------------------
struct Chord
{
//------------------------------------------------------------------------
	uint8 keyNote;		///< key note in chord
	uint8 rootNote;		///< lowest note in chord

	/** Bitmask of a chord.
	    1st bit set: minor second; 2nd bit set: major second, and so on. \n
		There is \b no bit for the keynote (root of the chord) because it is inherently always present. \n
		Examples:
		- XXXX 0000 0100 1000 (= 0x0048) -> major chord\n
		- XXXX 0000 0100 0100 (= 0x0044) -> minor chord\n
		- XXXX 0010 0100 0100 (= 0x0244) -> minor chord with minor seventh  */
	int16 chordMask;

	enum Masks {
		kChordMask = 0x0FFF,	///< mask for chordMask
		kReservedMask = 0xF000	///< reserved for future use
	};
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Audio processing context.
For each processing block the host provides timing information and
musical parameters that can change over time. For a host that supports jumps
(like cycle) it is possible to split up a processing block into multiple parts in
order to provide a correct project time inside of every block, but this behaviour
is not mandatory. Since the timing will be correct at the beginning of the next block
again, a host that is dependent on a fixed processing block size can choose to neglect
this problem.
\see IAudioProcessor, ProcessData*/
//------------------------------------------------------------------------
struct ProcessContext
{
//------------------------------------------------------------------------
	/** Transport state & other flags */
	enum StatesAndFlags
	{
		kPlaying          = 1 << 1,		///< currently playing
		kCycleActive      = 1 << 2,		///< cycle is active
		kRecording        = 1 << 3,		///< currently recording

		kSystemTimeValid  = 1 << 8,		///< systemTime contains valid information
		kContTimeValid    = 1 << 17,	///< continousTimeSamples contains valid information

		kProjectTimeMusicValid = 1 << 9,///< projectTimeMusic contains valid information
		kBarPositionValid = 1 << 11,	///< barPositionMusic contains valid information
		kCycleValid       = 1 << 12,	///< cycleStartMusic and barPositionMusic contain valid information

		kTempoValid       = 1 << 10,	///< tempo contains valid information
		kTimeSigValid     = 1 << 13,	///< timeSigNumerator and timeSigDenominator contain valid information
		kChordValid       = 1 << 18,	///< chord contains valid information

		kSmpteValid       = 1 << 14,	///< smpteOffset and frameRate contain valid information
		kClockValid       = 1 << 15		///< samplesToNextClock valid
	};

	uint32 state;					///< a combination of the values from \ref StatesAndFlags

	double sampleRate;				///< current sample rate (always valid)
	TSamples projectTimeSamples;	///< project time in samples (always valid)

	int64 systemTime;				///< system time in nanoseconds (optional)
	TSamples continousTimeSamples;	///< project time, without loop (optional)

	TQuarterNotes projectTimeMusic;	///< musical position in quarter notes (1.0 equals 1 quarter note)
	TQuarterNotes barPositionMusic;	///< last bar start position, in quarter notes
	TQuarterNotes cycleStartMusic;	///< cycle start in quarter notes
	TQuarterNotes cycleEndMusic;	///< cycle end in quarter notes

	double tempo;					///< tempo in BPM (Beats Per Minute)
	int32 timeSigNumerator;			///< time signature numerator (e.g. 3 for 3/4)
	int32 timeSigDenominator;		///< time signature denominator (e.g. 4 for 3/4)

	Chord chord;					///< musical info

	int32 smpteOffsetSubframes;		///< SMPTE (sync) offset in subframes (1/80 of frame)
	FrameRate frameRate;			///< frame rate

	int32 samplesToNextClock;		///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest)
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------

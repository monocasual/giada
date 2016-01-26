//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstmidicontrollers.h
// Created by  : Steinberg, 02/2006
// Description : VST MIDI Controller Enumeration
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

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
/** Controller Numbers (MIDI) */
//------------------------------------------------------------------------
enum ControllerNumbers
{
	kCtrlBankSelectMSB	=	0,	///< Bank Select MSB
	kCtrlModWheel		=	1,	///< Modulation Wheel
	kCtrlBreath			=	2,	///< Breath controller

	kCtrlFoot			=	4,	///< Foot Controller
	kCtrlPortaTime		=	5,	///< Portamento Time
	kCtrlDataEntryMSB	=	6,	///< Data Entry MSB
	kCtrlVolume			=	7,	///< Channel Volume (formerly Main Volume)
	kCtrlBalance		=	8,	///< Balance

	kCtrlPan			=	10,	///< Pan
	kCtrlExpression		=	11,	///< Expression
	kCtrlEffect1		=	12,	///< Effect Control 1
	kCtrlEffect2		=	13,	///< Effect Control 2

	//---General Purpose Controllers #1 to #4---
	kCtrlGPC1			=	16,	///< General Purpose Controller #1
	kCtrlGPC2			=	17,	///< General Purpose Controller #2
	kCtrlGPC3			=	18,	///< General Purpose Controller #3
	kCtrlGPC4			=	19,	///< General Purpose Controller #4

	kCtrlBankSelectLSB	=	32,	///< Bank Select LSB

	kCtrlDataEntryLSB	=	38,	///< Data Entry LSB

	kCtrlSustainOnOff	=	64,	///< Damper Pedal On/Off (Sustain)
	kCtrlPortaOnOff		=	65,	///< Portamento On/Off
	kCtrlSustenutoOnOff	=	66,	///< Sustenuto On/Off
	kCtrlSoftPedalOnOff	=	67,	///< Soft Pedal On/Off
	kCtrlLegatoFootSwOnOff=	68,	///< Legato Footswitch On/Off
	kCtrlHold2OnOff		=	69,	///< Hold 2 On/Off

	//---Sound Controllers #1 to #10---
	kCtrlSoundVariation	=	70, ///< Sound Variation
	kCtrlFilterCutoff	=	71,	///< Filter Cutoff (Timbre/Harmonic Intensity)
	kCtrlReleaseTime	=	72,	///< Release Time
	kCtrlAttackTime		=	73,	///< Attack Time
	kCtrlFilterResonance=	74,	///< Filter Resonance (Brightness)
	kCtrlDecayTime		=	75,	///< Decay Time
	kCtrlVibratoRate	=	76,	///< Vibrato Rate
	kCtrlVibratoDepth	=	77,	///< Vibrato Depth
	kCtrlVibratoDelay	=	78,	///< Vibrato Delay
	kCtrlSoundCtrler10	=	79, ///< undefined

	//---General Purpose Controllers #5 to #8---
	kCtrlGPC5			=	80,	///< General Purpose Controller #5
	kCtrlGPC6			=	81,	///< General Purpose Controller #6
	kCtrlGPC7			=	82,	///< General Purpose Controller #7
	kCtrlGPC8			=	83,	///< General Purpose Controller #8

	kCtrlPortaControl	=	84,	///< Portamento Control

	//---Effect Controllers---
	kCtrlEff1Depth		=	91,	///< Effect 1 Depth (Reverb Send Level)
	kCtrlEff2Depth		=	92,	///< Effect 2 Depth
	kCtrlEff3Depth		=	93,	///< Effect 3 Depth (Chorus Send Level)
	kCtrlEff4Depth		=	94,	///< Effect 4 Depth (Delay/Variation Level)
	kCtrlEff5Depth		=	95,	///< Effect 5 Depth

	kCtrlDataIncrement	=	96,	///< Data Increment (+1)
	kCtrlDataDecrement	=	97,	///< Data Decrement (-1)
	kCtrlNRPNSelectLSB 	=	98, ///< NRPN Select LSB
	kCtrlNRPNSelectMSB	=	99, ///< NRPN Select MSB
	kCtrlRPNSelectLSB	=	100, ///< RPN Select LSB
	kCtrlRPNSelectMSB	=	101, ///< RPN Select MSB

	//---Other Channel Mode Messages---
	kCtrlAllSoundsOff	=	120, ///< All Sounds Off
	kCtrlResetAllCtrlers =	121, ///< Reset All Controllers
	kCtrlLocalCtrlOnOff	=	122, ///< Local Control On/Off
	kCtrlAllNotesOff	=	123, ///< All Notes Off
	kCtrlOmniModeOff	=	124, ///< Omni Mode Off + All Notes Off
	kCtrlOmniModeOn		=	125, ///< Omni Mode On  + All Notes Off
	kCtrlPolyModeOnOff	=	126, ///< Poly Mode On/Off + All Sounds Off
	kCtrlPolyModeOn		=	127, ///< Poly Mode On

	//---Extra--------------------------
	kAfterTouch = 128,			///< After Touch
	kPitchBend,					///< Pitch Bend

	kCountCtrlNumber			///< Count of Controller Number
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/geoconstants.h
// Created by  : Steinberg, 11/2014
// Description : Defines orientations and directions as also used by fpoint.h and frect.h
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2014, Steinberg Media Technologies GmbH, All Rights Reserved
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
//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
enum Direction 
{
	kNorth,
	kNorthEast,
	kEast,
	kSouthEast,
	kSouth,
	kSouthWest,
	kWest,
	kNorthWest,
	kNoDirection,  //same position or center point of a geometry

	kNumberOfDirections
};

//------------------------------------------------------------------------
enum Orientation 
{
	kHorizontal,
	kVertical,
	kNumberOfOrientations
};

//------------------------------------------------------------------------
namespace GeoConstants {

//------------------------------------------------------------------------
inline Direction toOpposite (Direction dir) 
{
	switch (dir) 
	{
		case kNorth :		return kSouth;
		case kNorthEast :	return kSouthWest;
		case kEast :		return kWest;
		case kSouthEast :	return kNorthWest;
		case kSouth :		return kNorth;
		case kSouthWest :	return kNorthEast;
		case kWest :		return kEast;
		case kNorthWest :	return kSouthEast;
		case kNoDirection : return kNoDirection;
		default:
			return kNumberOfDirections;
	}
}

//------------------------------------------------------------------------
inline Orientation toOrientation (Direction dir) 
{
	switch (dir) 
	{
		case kNorth :		return kVertical;
		case kEast :		return kHorizontal;
		case kSouth :		return kVertical;
		case kWest :		return kHorizontal;
		default:
			return kNumberOfOrientations;
	}
}

//------------------------------------------------------------------------
inline Orientation toOrthogonalOrientation (Orientation dir) 
{
	switch (dir) 
	{
		case kVertical :	return kHorizontal;
		case kHorizontal :	return kVertical;
		default:
			return kNumberOfOrientations;
	}
}

//------------------------------------------------------------------------
} // namespace GeoConstants
} // namespace Steinberg

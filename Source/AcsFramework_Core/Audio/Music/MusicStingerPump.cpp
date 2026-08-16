// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Music/MusicStingerPump.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"

namespace
{
	/** 1 回で取り出す上限。取り出しが尽きないまま回り続けないための歯止め。 */
	constexpr usize kMaxPerCall = 8u;
}


usize CMusicStingerPump::ConsumeInto( CMusicDirector& Director, CAudioSubsystem& Audio ) noexcept
{
	usize Played = 0u;

	for ( usize Index = 0u; Index < kMaxPerCall; ++Index )
	{
		f32 Volume = 1.0f;
		const char* const Path = Director.ConsumeStinger( Volume );
		if ( Path == nullptr ) break;

		Audio.PlaySfx( FString( Path ), Volume );

		++Played;
		++m_PlayedCount;
	}

	return Played;
}

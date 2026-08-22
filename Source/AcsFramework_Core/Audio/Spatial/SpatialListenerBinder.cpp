// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialListenerBinder.h"

#include <cmath>

namespace
{
	/** 3成分が全て有限ならtrue。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


bool CSpatialListenerBinder::TryMakeFromCamera( const ALegacyScene3DAdapter& Scene, FAudioListener& OutListener ) noexcept
{
	FAudioListener Candidate;
	Candidate.position = Scene.Camera().Eye();
	if ( !IsFinite( Candidate.position ) ) return false;

	if ( const FScene3DCameraState* const Authored = Scene.AuthoredCamera() )
	{
		Candidate.forward = Authored->Forward;
		Candidate.up = Authored->Up;
	}
	else
	{
		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D Snapshot;
		if ( !Scene.TryCaptureOrbitCameraSnapshot( Snapshot ) ) return false;
		Candidate.forward = Snapshot.current.target - Candidate.position;
		Candidate.up = FVec3::Up();
	}

	if ( !IsFinite( Candidate.forward ) || !IsFinite( Candidate.up ) ) return false;
	const f32 ForwardLengthSq = LengthSq( Candidate.forward );
	const f32 UpLengthSq = LengthSq( Candidate.up );
	const f32 RightLengthSq = LengthSq( Cross( Candidate.forward, Candidate.up ) );
	if ( !std::isfinite( ForwardLengthSq ) || ForwardLengthSq <= 0.000001f ) return false;
	if ( !std::isfinite( UpLengthSq ) || UpLengthSq <= 0.000001f ) return false;
	if ( !std::isfinite( RightLengthSq ) || RightLengthSq <= 0.000001f ) return false;

	Candidate.forward = Normalize( Candidate.forward );
	Candidate.up = Normalize( Candidate.up );
	if ( !IsFinite( Candidate.forward ) || !IsFinite( Candidate.up ) ) return false;

	OutListener = Candidate;
	return true;
}


FAudioListener CSpatialListenerBinder::MakeListener() const noexcept
{
	if ( m_Target == nullptr ) return m_Manual;

	const FTransform3D World = m_Target->World();

	FAudioListener Listener;
	Listener.position = World.position;
	Listener.forward = Rotate( World.rotation, FVec3::Forward() );
	Listener.up = Rotate( World.rotation, FVec3::Up() );

	return Listener;
}

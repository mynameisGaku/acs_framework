// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialListenerBinder.h"


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

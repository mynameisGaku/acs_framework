// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Node3D/NodeWorldMovement3D.h"

#include <cmath>

namespace
{
	/** 3成分がすべて有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


bool TryCalculateLocalPositionAfterWorldTranslation3D( const ANode& Node, FVec3 WorldTranslation, FVec3& OutLocalPosition ) noexcept
{
	if ( Node.IsPendingDestroy() || !IsFinite( WorldTranslation ) ) return false;

	/** 移動前のノード原点のworld位置。 */
	const FVec3 CurrentWorldPosition = Node.World().position;
	if ( !IsFinite( CurrentWorldPosition ) ) return false;

	/** 指定したworld移動を加えたノード原点の位置。 */
	const FVec3 TargetWorldPosition = CurrentWorldPosition + WorldTranslation;
	if ( !IsFinite( TargetWorldPosition ) ) return false;

	/** 親が無い場合はworld位置と一致する適用先ローカル位置。 */
	FVec3 LocalPosition = TargetWorldPosition;
	if ( const ANode* const Parent = Node.Parent() )
	{
		LocalPosition = TransformPoint( TargetWorldPosition, Inverse( Parent->World().ToMat4() ) );
	}
	if ( !IsFinite( LocalPosition ) ) return false;

	OutLocalPosition = LocalPosition;
	return true;
}


bool TryTranslateNodeWorld3D( ANode& Node, FVec3 WorldTranslation ) noexcept
{
	/** 検証済みworld移動を適用するためのローカル位置。 */
	FVec3 LocalPosition;
	if ( !TryCalculateLocalPositionAfterWorldTranslation3D( Node, WorldTranslation, LocalPosition ) ) return false;

	Node.SetPosition( LocalPosition );
	return true;
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DUpdateResult.h"

namespace
{
	/** 配列に同じ世代付きノード識別子があればtrue。 */
	bool ContainsNode_Internal( const TArray<FNodeId>& Nodes, FNodeId Node ) noexcept
	{
		for ( const FNodeId Candidate : Nodes )
		{
			if ( Candidate == Node ) return true;
		}
		return false;
	}
}


bool FProximityTrigger3DUpdateResult::DidEnter( FNodeId Node ) const noexcept
{
	return ContainsNode_Internal( EnteredNodes, Node );
}


bool FProximityTrigger3DUpdateResult::IsInside( FNodeId Node ) const noexcept
{
	return ContainsNode_Internal( InsideNodes, Node );
}


bool FProximityTrigger3DUpdateResult::DidExit( FNodeId Node ) const noexcept
{
	return ContainsNode_Internal( ExitedNodes, Node );
}

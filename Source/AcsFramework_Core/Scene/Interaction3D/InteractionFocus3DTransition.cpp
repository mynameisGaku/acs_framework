// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DTransition.h"

FInteractionFocus3DUpdateResult AdvanceInteractionFocus3D( const FInteractionFocus3DState& State, const FInteractionFocus3DInput& Input ) noexcept
{
	FInteractionFocus3DUpdateResult Result;
	Result.PreviousNode = State.FocusedNode;
	Result.FocusedNode = Input.CandidateNode;
	if ( Input.bActivateRequested && Input.CandidateNode.IsValid() ) Result.ActivatedNode = Input.CandidateNode;
	return Result;
}

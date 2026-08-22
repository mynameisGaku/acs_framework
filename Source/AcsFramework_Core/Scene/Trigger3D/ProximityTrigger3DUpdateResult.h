// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 球型近接トリガーの前回から今回への進入、滞在、退出。 */
struct FProximityTrigger3DUpdateResult
{
	/** 今回初めて範囲内になった世代付きノード識別子。 */
	TArray<FNodeId> EnteredNodes;

	/** 今回の更新後に範囲内にいる世代付きノード識別子。 */
	TArray<FNodeId> InsideNodes;

	/** 前回は範囲内で、今回は範囲外になった世代付きノード識別子。 */
	TArray<FNodeId> ExitedNodes;

	/** 指定ノードが今回進入したならtrue。 */
	bool DidEnter( FNodeId Node ) const noexcept;

	/** 指定ノードが今回の更新後も範囲内ならtrue。 */
	bool IsInside( FNodeId Node ) const noexcept;

	/** 指定ノードが今回退出したならtrue。 */
	bool DidExit( FNodeId Node ) const noexcept;
};

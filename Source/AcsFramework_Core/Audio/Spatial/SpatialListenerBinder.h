// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 聴く位置を、追いかける相手から作る係。
 *
 * @details
 * 「毎フレーム自分でノードの位置を読んで渡す」のを各所に書かせないためのもの。相手を差して
 * おけば、そのノードの世界位置と向きから聴く位置を組み立てる。
 *
 * 相手を差していないときは、直接指定された値をそのまま返す (カメラを持たない 2D の遊びなど)。
 */
class CSpatialListenerBinder
{
public:
	/**
	 * 追いかける相手を差す。
	 *
	 * @param Node 追いかけるノード。nullptr を渡すと追いかけるのをやめる。
	 */
	void SetTarget( ANode* Node ) noexcept { m_Target = Node; }

	/** 追いかけている相手を返す。 */
	ANode* GetTarget() const noexcept { return m_Target; }

	/**
	 * 追いかけずに、聴く位置を直接決める。
	 *
	 * @param Listener 聴く位置と向き。
	 */
	void SetManualListener( const FAudioListener& Listener ) noexcept { m_Manual = Listener; }

	/**
	 * いまの聴く位置を組み立てる。
	 *
	 * @return 聴く位置と向き。
	 */
	FAudioListener MakeListener() const noexcept;

private:
	/** 追いかける相手。所有はしない。 */
	ANode* m_Target = nullptr;

	/** 相手を差していないときに使う値。 */
	FAudioListener m_Manual;
};

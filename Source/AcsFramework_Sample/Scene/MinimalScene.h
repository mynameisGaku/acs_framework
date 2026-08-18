// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * **この枠組みで «一番少ない手数» はここまで、を示す場面。**
 *
 * @details
 * `Demo3DScene` は色々を «触れること» を見せるための場面なので長い。こちらは逆で、
 * **何も設定しないと何が出るか**を測るためのもの。
 *
 * 空・太陽・大気・雲・影・環境光・遮蔽・反射・トーンマップは、1 行も書かなくても効く。
 * ここに書いてあるのは «何をどこに置くか» だけ。
 *
 * 起動場面を差し替えるには `CAcsFrameworkApp::CreateInitialScene` を override する。
 */
class AMinimalScene : public ALegacyScene3DAdapter
{
public:
	/** 物を置いて、カメラを引く。 */
	void OnEnter() noexcept override;

	/**
	 * 置いた物を回す。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

private:
	/** 回す対象。所有はしない (木が持っている)。 */
	ANode* m_Spinner = nullptr;
};

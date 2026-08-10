// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoadRequest.h"

using namespace acs;

class CAssetLoaderSubsystem;

/** 非所有のLoaderに対する要求の追跡と終了時の取消しを行う通常型。Loaderが破棄済みの場合は利用できない。 */
class CAssetLoadScope final
{
public:
	/** GameInstance寿命のLoaderを非所有で参照する。進行中のBeginが返るまで、callbackでscopeが破棄される場合もLoaderを生存させる。内部状態の確保に失敗した場合は空状態で構築する。 */
	explicit CAssetLoadScope( CAssetLoaderSubsystem& Loader ) noexcept;

	/** 要求の複製を禁止し、取消し対象の一意性を保つ。 */
	CAssetLoadScope( const CAssetLoadScope& ) = delete;

	/** 要求の複製を禁止し、取消し対象の一意性を保つ。 */
	CAssetLoadScope& operator=( const CAssetLoadScope& ) = delete;

	/** 要求の移動を禁止し、非所有Loader参照の寿命を固定する。 */
	CAssetLoadScope( CAssetLoadScope&& ) = delete;

	/** 要求の移動を禁止し、非所有Loader参照の寿命を固定する。 */
	CAssetLoadScope& operator=( CAssetLoadScope&& ) = delete;

	/** 追跡中の自身の要求だけを取消して終了する。Loaderは追従中に生存し、内部状態の確保失敗時は何もしない。 */
	~CAssetLoadScope() noexcept;

	/**
	 * Pathsは読み込むpath列、OnCompleteは完了通知を表す。
	 * 同期callback、入れ子のBegin・CancelAll、Loader直呼出しの再入では後から変わった状態を優先する。
	 * callbackでscopeが破棄されても、Loaderは進行中のBeginが返るまで生存し、内部状態で外部呼出しを完了する。
	 * 無効な返却値は旧要求が現在かつ処理中なら追跡を戻し、返却値は完了済みでもIsActiveの追跡対象とは分ける。内部状態の確保失敗時は無効値を返す。
	 */
	FAssetLoadRequest Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete = FSimpleDelegate() );

	/** 無効・外部要求はfalseで状態を変えず、所有要求は先に切り離してLoaderの実取消結果だけを返す。完了・置換済みならfalseになり得る。 */
	bool Cancel( FAssetLoadRequest Request ) noexcept;

	/** 所有要求を先に切り離してから個別取消しする。内部状態の確保失敗時は何もしない。 */
	void CancelAll() noexcept;

	/** 自身の要求、Loaderの現在要求、処理中状態が一致するときだけtrueを返し、内部状態の確保失敗時はfalseを返す。 */
	bool IsActive( FAssetLoadRequest Request ) const noexcept;

private:
	/** Loader、所有要求、世代をscope破棄後まで保持する内部寿命状態。 */
	struct FState;

	/** 再入処理より前の追跡結果を無効にする世代を進める。世代0は使用しない。 */
	static void AdvanceEpoch( FState& State ) noexcept;

	/** Loader、所有要求、世代を共有し、scope破棄後も外部呼出しの後処理を保持する内部状態。 */
	TSharedPtr<FState> m_State;
};

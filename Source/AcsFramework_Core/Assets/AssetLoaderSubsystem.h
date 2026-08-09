// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoadBatch.h"

using namespace acs;

/** GameInstanceで1件のアセット読み込み単位を共有する窓口。 */
class CAssetLoaderSubsystem : public ASubsystem
{
public:
	/** GameInstance用サブシステムの登録情報を定義する。 */
	ACS_SUBSYSTEM_KIND( CAssetLoaderSubsystem )

	/** 読み込みに使うレジストリを設定する。レジストリはこのサブシステムより長く生存する。 */
	void Bind( CAssetRegistry& Registry ) noexcept { m_Registry = &Registry; }

	/** Pathsを入力順で受け付け、全入力完了時にOnCompleteを1回呼び出す。失敗入力も件数を保つ。 */
	void Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete = FSimpleDelegate() );

	/** 現在の読み込み単位の観測と通知を解除する。エンジンの処理とキャッシュは停止しない。 */
	void Cancel() noexcept;

	/** 現batchが処理中かを返す。 */
	bool IsLoading() const noexcept { return m_Batch.IsLoading(); }

	/** 完了件数を入力件数で割った進捗を返す。空または失敗完了は1を返す。 */
	f32 GetProgress() const noexcept;

	/** 現batchに失敗entryがあるかを返す。 */
	bool HasFailed() const noexcept { return m_Batch.HasFailed(); }

	/** Beginへ渡した入力件数を返す。確保失敗時も入力件数を保つ。 */
	usize Num() const noexcept { return m_Batch.Num(); }

	/** Begin入力のIndexに対応する成功assetを返す。範囲外または失敗時は空を返す。 */
	TSharedPtr<AAsset> GetAsset( usize Index ) const noexcept;

	/** 1フレーム分の非同期結果を確認し、完了時に通知を配送する。 */
	void Update() noexcept;

private:
	/** エンジンのレジストリを所有せず参照する。 */
	CAssetRegistry* m_Registry = nullptr;

	/** 現在観測している1件のbatch。 */
	FAssetLoadBatch m_Batch;
};

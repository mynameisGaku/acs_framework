// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoadBatch.h"
#include "AcsFramework_Core/Assets/AssetLoadRequest.h"

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

	/**
	 * 読み込みを開始し、発行元と世代を含む要求を返す。
	 *
	 * @param Paths 入力順を保つアセットパス。
	 * @param OnComplete 読み込み単位の完了時に呼び出す通知。即時完了時は開始中に呼び出される。
	 * @return 受理した要求。発行元または世代の確保に失敗した場合は無効値を返し、既存状態を変えない。
	 * @details 要求は発行元のLoaderと組み合わせて照合する。完了通知内の再入で新しい要求が始まっても、外側の処理は新しい状態へ触れない。
	 */
	FAssetLoadRequest BeginRequest( const TArray<FString>& Paths, FSimpleDelegate OnComplete = FSimpleDelegate() );

	/** BeginRequestで開始した現在の要求を返す。既存のBeginで開始した処理では無効値を返す。 */
	FAssetLoadRequest GetCurrentRequest() const noexcept;

	/**
	 * RequestがこのLoaderの現在の読み込みを指すかを返す。
	 *
	 * @param Request 照合する要求。
	 * @return 発行元と世代が完全一致する処理中または完了済みの要求ならtrue、それ以外はfalse。
	 * @details 無効値や別Loaderの要求はfalseで、状態を変えない。
	 */
	bool IsCurrent( FAssetLoadRequest Request ) const noexcept;

	/**
	 * 現在の処理中要求だけを取り消す。
	 *
	 * @param Request 取り消す要求。
	 * @return 現在の処理中要求と完全一致して取り消した場合はtrue、それ以外はfalse。
	 * @details 古い要求、無効値、完了済みの要求では完了通知、結果、現在値を変えない。
	 */
	bool CancelRequest( FAssetLoadRequest Request ) noexcept;

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
	/** プロセス全体で再利用しない発行元識別子と、読み込み窓口内で再利用しない世代から要求を作る。 */
	FAssetLoadRequest AcquireRequest() noexcept;

	/** エンジンのレジストリを所有せず参照する。 */
	CAssetRegistry* m_Registry = nullptr;

	/** 現在観測している1件のbatch。 */
	FAssetLoadBatch m_Batch;

	/** このLoaderだけを識別するプロセス全体の非0値。 */
	u64 m_OwnerId = 0u;

	/** 次に発行する世代。0になった後は要求を発行しない。 */
	u64 m_NextGeneration = 1u;

	/** 現在の読み込み単位へ対応する要求。完了後も結果とともに保持する。 */
	FAssetLoadRequest m_CurrentRequest;
};

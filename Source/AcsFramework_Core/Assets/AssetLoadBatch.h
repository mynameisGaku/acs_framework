// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CAssetLoaderSubsystem;

/** 1回分のアセット読み込みと、その完了結果を保持する通常型。 */
class FAssetLoadBatch
{
public:
	/** 空の読み込み単位を構築する。実際の読み込みはGameInstanceの窓口から開始する。 */
	FAssetLoadBatch() noexcept = default;

	/** 1件の観測状態を持つため複製しない。 */
	FAssetLoadBatch( const FAssetLoadBatch& ) = delete;

	/** 1件の観測状態を持つため複製しない。 */
	FAssetLoadBatch& operator=( const FAssetLoadBatch& ) = delete;

	/** サブシステム内の固定所有物として移動しない。 */
	FAssetLoadBatch( FAssetLoadBatch&& ) = delete;

	/** サブシステム内の固定所有物として移動しない。 */
	FAssetLoadBatch& operator=( FAssetLoadBatch&& ) = delete;

private:
	/** エンジンへ渡すパスの最大UTF-16文字数。終端NULを含む。 */
	static constexpr usize kMaxAssetPathLength = 260u;

	/** 入力1件に対応するパス、非同期結果、読み込み結果を保持する。 */
	struct FEntry
	{
		/** エンジンへ渡す前に確保したUTF-8パス。 */
		FString Path;

		/** エンジンが返した非同期結果。 */
		FAssetFuture Future;

		/** この入力の完了を記録する。 */
		bool bFinished = false;

		/** この入力の失敗を記録する。 */
		bool bFailed = false;

		/** 成功時のアセット。 */
		TSharedPtr<AAsset> Asset;
	};

	/** 旧状態の観測を外し、新しい完了通知を受け付ける。 */
	void Start( CAssetRegistry* Registry, const TArray<FString>& Paths, FSimpleDelegate OnComplete ) noexcept;

	/** 読み込み処理を止めずに現状態の観測と完了通知を解除する。 */
	void Cancel() noexcept;

	/** 完了した非同期結果を調べ、全入力完了時に通知を1回配送する。 */
	void Update() noexcept;

	/** すべての入力を失敗完了として通知を配送する。 */
	void SetFailureSummary() noexcept;

	/** 状態を先に完了へ畳み、完了通知をローカルへ移して呼び出す。 */
	void Finish() noexcept;

	/** 完了通知と入力一覧を破棄して空状態へ戻す。 */
	void ResetObservation() noexcept;

	/** 現batchの処理中状態を返す。 */
	bool IsLoading() const noexcept { return m_bLoading; }

	/** 完了件数を要求件数で割った進捗を返す。 */
	f32 GetProgress() const noexcept;

	/** 失敗entryがあるかを返す。 */
	bool HasFailed() const noexcept { return m_bFailed; }

	/** Begin入力の件数を返す。確保失敗時も入力件数を保持する。 */
	usize Num() const noexcept { return m_RequestedCount; }

	/** Begin入力の添字に対応する成功assetを返す。 */
	TSharedPtr<AAsset> GetAsset( usize Index ) const noexcept;

	/** GameInstanceの窓口だけが読み込み単位を操作する。 */
	friend class CAssetLoaderSubsystem;

	/** Begin入力順のentry配列。 */
	TArray<FEntry> m_Entries;

	/** 完了時に1回だけ呼び出すcallback。 */
	FSimpleDelegate m_OnComplete;

	/** Beginで受け付けた入力件数。 */
	usize m_RequestedCount = 0;

	/** 完了したentry件数。 */
	usize m_FinishedCount = 0;

	/** 現batchが処理中かを示す。 */
	bool m_bLoading = false;

	/** 失敗entryが1件以上あるかを示す。 */
	bool m_bFailed = false;
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Assets/AssetLoadRequest.h"

using namespace acs;

class CAssetLoaderSubsystem;
class CLoadingScreenSubsystem;

/** 構築から破棄まで追従解除を所有するLoadingScreen用の局所通常型。 */
class CLoadingScreenFollowScope final
{
public:
	/** LoadingScreenはscope全期間、LoaderはFollow成功からResetまたは自動完了まで参照可能にして非所有で束ねる。 */
	explicit CLoadingScreenFollowScope( CLoadingScreenSubsystem& Loading ) noexcept;

	/** 所有要求を追従解除する。外部置換後は新しい追従を変更しない。 */
	~CLoadingScreenFollowScope() noexcept;

	/** コピーによる二重解除を防ぐ。 */
	CLoadingScreenFollowScope( const CLoadingScreenFollowScope& ) = delete;
	/** コピー代入による二重解除を防ぐ。 */
	CLoadingScreenFollowScope& operator=( const CLoadingScreenFollowScope& ) = delete;
	/** 移動による解除所有の分散を防ぐ。 */
	CLoadingScreenFollowScope( CLoadingScreenFollowScope&& ) = delete;
	/** 移動代入による解除所有の分散を防ぐ。 */
	CLoadingScreenFollowScope& operator=( CLoadingScreenFollowScope&& ) = delete;

	/** 現在のLoader要求を追従させる。無効または現在でない要求ではfalseを返し、以前の所有を保持する。 */
	bool Follow( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message = FString() );

	/** 所有要求を先に無効化する。現在の要求・追従世代と一致して解除できた場合はtrue、無効・外部変更後はfalseを返す。 */
	bool Reset() noexcept;

	/** 要求・世代がsubsystemの現在追従と一致する所有状態か返す。無効または外部変更後はfalseを返す。 */
	bool Owns( FAssetLoadRequest Request ) const noexcept;

	/** 現在の追従を所有している場合だけRequestを返し、それ以外は無効値を返す。 */
	FAssetLoadRequest GetRequest() const noexcept;

private:
	/** LoadingScreenへの非所有参照を保持する。LoadingScreenはscopeより長く生存する。 */
	CLoadingScreenSubsystem* m_Loading = nullptr;

	/** このscopeが解除対象として保持するRequest。 */
	FAssetLoadRequest m_Request;

	/** subsystemの現在追従と結び付く非0の追従世代。 */
	u64 m_Revision = 0u;
};

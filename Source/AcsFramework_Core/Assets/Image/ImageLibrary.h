// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * `Assets`に置いた画像を読み、CPU画像アセットとして返す。
 *
 * @details 画像のデコードと同一パスのキャッシュはACSの`CAssetRegistry`へ任せる。
 * この型は`Assets`相対名の解決、対応形式の事前確認、失敗理由の記録だけを担う。
 */
class CImageLibrary
{
public:
	/**
	 * 読み込みに使う登録簿を渡す。
	 *
	 * @param Registry `CApplication::GetAssets()`。
	 */
	void Bind( CAssetRegistry& Registry ) noexcept { m_Registry = &Registry; }

	/** 登録簿が渡され、読み込み可能な状態ならtrueを返す。 */
	bool IsBound() const noexcept { return m_Registry != nullptr; }

	/**
	 * `Assets`から画像を同期読み込みする。
	 *
	 * @param RelativePath `Assets`からの相対名。
	 * @return 読み込んだ`AImageAsset`。未接続、形式不正、パス不正、読込失敗では空。
	 */
	TSharedPtr<AAsset> Load( FStringView RelativePath ) noexcept;

	/**
	 * ACSの標準画像ローダが扱う拡張子ならtrueを返す。
	 *
	 * @param RelativePath 判定する画像名。
	 */
	static bool IsSupported( FStringView RelativePath ) noexcept;

private:
	/** 読み込みを頼む登録簿。所有しない。 */
	CAssetRegistry* m_Registry = nullptr;
};

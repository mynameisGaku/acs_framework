// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/Localization/LocaleCatalog.h"
#include "AcsFramework_Core/Text/Localization/LocaleChangeBroadcaster.h"
#include "AcsFramework_Core/Text/Localization/LocaleName.h"
#include "AcsFramework_Core/Text/Localization/LocalizationTableFile.h"
#include "AcsFramework_Core/Text/Localization/LocalizationTableParser.h"
#include "AcsFramework_Core/Text/Localization/TextFormatter.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 引いた文が期待どおりかを見る。 */
	bool TextIs( const char* Found, const char* Expected ) noexcept
	{
		if ( Found == nullptr ) return false;

		return FStringView( Found ) == FStringView( Expected );
	}

	/** 組み立てた文が期待どおりかを見る。 */
	bool BuiltIs( const FString& Built, const char* Expected ) noexcept
	{
		return Built.View() == FStringView( Expected );
	}

	/** 何回呼ばれたかを数えるだけの相手。 */
	class CCountingListener final : public ILocaleChangeListener
	{
	public:
		void OnLocaleChanged( ELocale Locale ) noexcept override
		{
			++CallCount;
			LastLocale = Locale;
		}

		usize CallCount = 0u;
		ELocale LastLocale = ELocale::Default;
	};
}


void RunLocalizationTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CLocaleCatalog / 言語ごとに引く" );

	{
		CLocaleCatalog Catalog;

		Harness.Check( Catalog.Register( ELocale::Ja, FString( "ui.start" ), FString( "はじめる" ) ), "足せる" );
		Harness.Check( Catalog.Register( ELocale::En, FString( "ui.start" ), FString( "Start" ) ), "別の言語も足せる" );

		Catalog.SetLocale( ELocale::Ja );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.start" ) ), "はじめる" ), "いまの言語で引ける" );

		Catalog.SetLocale( ELocale::En );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.start" ) ), "Start" ), "切り替えると変わる" );

		Harness.Check( TextIs( Catalog.FindForLocale( ELocale::Ja, FString( "ui.start" ) ), "はじめる" ),
			"言語を指しても引ける" );

		Harness.Check( Catalog.Has( FString( "ui.start" ) ), "在ることを確かめられる" );
		Harness.Check( !Catalog.Has( FString( "ui.nothing" ) ), "無いものは無いと分かる" );
		Harness.CheckEqualU64( Catalog.KeyCount( ELocale::Ja ), 1u, "鍵の数" );

		Harness.Check( !Catalog.Register( ELocale::Ja, FString(), FString( "空の鍵" ) ), "鍵が空なら足さない" );
	}

	Harness.BeginSuite( "CLocaleCatalog / 引けないときの落とし込み" );

	{
		CLocaleCatalog Catalog;
		Catalog.Register( ELocale::En, FString( "ui.only_en" ), FString( "English only" ) );

		Catalog.SetLocale( ELocale::Ja );

		// 訳が無い言語では、既定 (En) の文へ落ちる。ここが空文字だと画面から文字が消える。
		Harness.Check( TextIs( Catalog.Find( FString( "ui.only_en" ) ), "English only" ), "既定の言語へ落ちる" );

		// どこにも無ければ鍵がそのまま出る。画面に鍵が出ていたら «表に無い» ということ。
		Harness.Check( TextIs( Catalog.Find( FString( "ui.missing" ) ), "ui.missing" ), "無ければ鍵がそのまま出る" );
	}

	Harness.BeginSuite( "CLocaleCatalog / 渡した文字列が消えても壊れない" );

	{
		CLocaleCatalog Catalog;

		// エンジンの辞書は渡された const char* を複製しない。写しを持たずに登録すると、
		// ここで元が消えた時点で «しばらく動いた後で突然おかしな文字が出る» 壊れ方をする。
		{
			FString TemporaryKey( "ui.temporary" );
			FString TemporaryText( "消える予定の文" );

			Harness.Check( Catalog.Register( ELocale::Ja, TemporaryKey, TemporaryText ), "一時的な文字列でも足せる" );
		}

		Catalog.SetLocale( ELocale::Ja );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.temporary" ) ), "消える予定の文" ),
			"元が消えた後でも引ける (写しを持っている)" );

		Catalog.Clear();
		Harness.CheckEqualU64( Catalog.KeyCount( ELocale::Ja ), 0u, "捨てられる" );
	}

	Harness.BeginSuite( "CLocaleName / 言語の名前を読む" );

	{
		ELocale Locale = ELocale::Default;

		Harness.Check( CLocaleName::TryParse( FStringView( "ja" ), Locale ) && Locale == ELocale::Ja, "小文字" );
		Harness.Check( CLocaleName::TryParse( FStringView( "JA" ), Locale ) && Locale == ELocale::Ja, "大文字" );
		Harness.Check( CLocaleName::TryParse( FStringView( "Ja" ), Locale ) && Locale == ELocale::Ja, "列挙子そのまま" );

		Harness.Check( CLocaleName::TryParse( FStringView( "zh-cn" ), Locale ) && Locale == ELocale::ZhCn, "区切り付き" );
		Harness.Check( CLocaleName::TryParse( FStringView( "zh_cn" ), Locale ) && Locale == ELocale::ZhCn, "下線でも同じ" );
		Harness.Check( CLocaleName::TryParse( FStringView( "zhcn" ), Locale ) && Locale == ELocale::ZhCn, "区切り無しでも同じ" );

		// 取り違えると別の言語が出る。zh-cn と zh-tw は 1 文字しか違わない。
		Harness.Check( CLocaleName::TryParse( FStringView( "zh-tw" ), Locale ) && Locale == ELocale::ZhTw, "似た名前を混同しない" );

		Harness.Check( !CLocaleName::TryParse( FStringView( "klingon" ), Locale ), "知らない名前は読まない" );
		Harness.Check( !CLocaleName::TryParse( FStringView( "" ), Locale ), "空は読まない" );

		Harness.Check( CLocaleName::ToText( ELocale::ZhCn ) == FStringView( "ZhCn" ), "名前を出せる" );
		Harness.Check( CLocaleName::ToText( ELocale::Default ) == FStringView( "En" ), "既定は En と出る" );
	}

	Harness.BeginSuite( "CLocalizationTableParser / 表を読む" );

	{
		CLocaleCatalog Catalog;

		const char* const Table =
			"# 覚え書きの行\n"
			"\n"
			"[ja]\n"
			"ui.start = はじめる\n"
			"ui.title=冒険の書\n"
			"ui.empty =\n"
			"\n"
			"[en]\n"
			"ui.start = Start\n"
			"ui.title = Adventure\n";

		const FLocalizationParseResult Result = CLocalizationTableParser::ParseInto( Catalog, FStringView( Table ) );

		Harness.Check( Result.Succeeded(), "落とさずに読める" );
		Harness.CheckEqualU64( Result.Registered, 5u, "読めた行の数" );
		Harness.CheckEqualU64( Result.Skipped, 0u, "落とした行は無い" );

		Catalog.SetLocale( ELocale::Ja );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.start" ) ), "はじめる" ), "空白付きの行" );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.title" ) ), "冒険の書" ), "空白無しの行" );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.empty" ) ), "" ), "わざと空にした文は空のまま" );

		Catalog.SetLocale( ELocale::En );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.title" ) ), "Adventure" ), "節が切り替わっている" );
	}

	Harness.BeginSuite( "CLocalizationTableParser / 読めない行を数える" );

	{
		CLocaleCatalog Catalog;

		const char* const Table =
			"[ja]\n"
			"= 鍵が無い\n"
			"区切りの無い行\n"
			"ui.ok = 大丈夫\n"
			"[klingon]\n"
			"ui.ignored = 知らない言語\n";

		const FLocalizationParseResult Result = CLocalizationTableParser::ParseInto( Catalog, FStringView( Table ) );

		Harness.Check( !Result.Succeeded(), "落とした行があると分かる" );
		Harness.CheckEqualU64( Result.Registered, 1u, "読めた行だけ入る" );

		// 鍵無し / 区切り無し / 知らない言語の見出し で 3。知らない節の中身は数に入れない。
		Harness.CheckEqualU64( Result.Skipped, 3u, "落とした行の数" );

		Catalog.SetLocale( ELocale::Ja );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.ok" ) ), "大丈夫" ), "落とした行の後も読み続ける" );
		Harness.Check( !Catalog.Has( FString( "ui.ignored" ) ), "知らない言語の中身は入らない" );
	}

	Harness.BeginSuite( "CLocalizationTableParser / 見出しより前の行" );

	{
		CLocaleCatalog Catalog;

		const char* const Table =
			"ui.orphan = どの言語か決まらない\n"
			"[ja]\n"
			"ui.ok = 大丈夫\n";

		const FLocalizationParseResult Result = CLocalizationTableParser::ParseInto( Catalog, FStringView( Table ) );

		Harness.Check( Result.bMissingLocaleHeader, "見出しより前の行に気付く" );
		Harness.CheckEqualU64( Result.Registered, 1u, "見出しの後は読める" );
		Harness.Check( !Catalog.Has( FString( "ui.orphan" ) ), "決まらない行は入れない" );
	}

	Harness.BeginSuite( "CLocalizationTableFile / AssetsからUTF-8表を読む" );

	{
		/** 試験用Assets内で訳文表を置くフォルダ。 */
		constexpr const wchar_t* kTableDirectory = L"TestOutput\\LocalizationAssets\\Text";
		/** 書き込みと後片付けに使う訳文表の実パス。 */
		constexpr const wchar_t* kTablePath = L"TestOutput\\LocalizationAssets\\Text\\game.loc";
		/** `CAssetRoot`へ一時指定する試験用素材ルート。 */
		constexpr const char* kAssetRoot = "TestOutput/LocalizationAssets";
		/** BOM、日本語、英語を含むUTF-8訳文表。 */
		const char TableBytes[] =
			"\xef\xbb\xbf"
			"[ja]\n"
			"ui.start = はじめる\n"
			"[en]\n"
			"ui.start = Start\n";

		/** 試験用素材フォルダの作成結果。 */
		const TResult<void> Directory = CFileSystem::CreateDirectory( kTableDirectory );
		Harness.Check( Directory.IsOk(), "試験用Assetsを作れる" );
		/** BOM付き表の書き込み結果。 */
		const TResult<void> Written = CFileSystem::WriteAllBytes(
			kTablePath, reinterpret_cast<const byte*>( TableBytes ), sizeof( TableBytes ) - 1u );
		Harness.Check( Written.IsOk(), "BOM付きUTF-8表を置ける" );

		CAssetRoot::Override( FStringView( kAssetRoot ) );
		/** ファイルから登録された文を持つ試験用辞書。 */
		CLocaleCatalog Catalog;
		/** 相対パスから訳文表を読んだ結果。 */
		TResult<FLocalizationParseResult> Loaded = CLocalizationTableFile::LoadInto(
			Catalog, FStringView( "Text/game.loc" ) );
		Harness.Check( Loaded.IsOk(), "Assetsから相対名だけで読める" );
		if ( Loaded.IsOk() )
		{
			Harness.Check( Loaded.Value().Succeeded(), "BOMを表の一部として誤解析しない" );
			Harness.CheckEqualU64( Loaded.Value().Registered, 2u, "両言語を足す" );
		}

		Catalog.SetLocale( ELocale::Ja );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.start" ) ), "はじめる" ), "日本語を引ける" );
		Catalog.SetLocale( ELocale::En );
		Harness.Check( TextIs( Catalog.Find( FString( "ui.start" ) ), "Start" ), "英語を引ける" );

		/** 失敗時に維持されるべき既存の英語鍵数。 */
		const usize EnglishCount = Catalog.KeyCount( ELocale::En );
		/** 素材ルート外を指す入力の拒否結果。 */
		const TResult<FLocalizationParseResult> Traversal = CLocalizationTableFile::LoadInto(
			Catalog, FStringView( "../outside.loc" ) );
		Harness.Check( Traversal.IsErr(), "Assetsの外は読まない" );
		/** 存在しない表の読み込み結果。 */
		const TResult<FLocalizationParseResult> Missing = CLocalizationTableFile::LoadInto(
			Catalog, FStringView( "Text/missing.loc" ) );
		Harness.Check( Missing.IsErr(), "無いファイルは失敗として返す" );
		Harness.CheckEqualU64( Catalog.KeyCount( ELocale::En ), EnglishCount, "読み込み失敗で既存の文を変えない" );

		CAssetRoot::Override( FStringView() );
		/** 試験用ファイルだけを削除した結果。 */
		const TResult<void> Deleted = CFileSystem::Delete( kTablePath );
		Harness.Check( Deleted.IsOk(), "試験用ファイルを片付けられる" );
	}

	Harness.BeginSuite( "CTextFormatter / 値を差し込む" );

	{
		const FTextArgument Args[] =
		{
			FTextArgument::FromText( FStringView( "スライム" ) ),
			FTextArgument::FromInteger( 12 ),
		};

		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{0} に {1} のダメージ" ), Args, 2u ),
			"スライム に 12 のダメージ" ), "順番どおり" );

		// 言語によって語順が変わる。番号で指すのはこのため。
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{1} damage to {0}" ), Args, 2u ),
			"12 damage to スライム" ), "並びが入れ替わってもよい" );

		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{0} と {0}" ), Args, 2u ),
			"スライム と スライム" ), "同じ番号を二度使える" );

		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "差し込み無し" ), Args, 2u ),
			"差し込み無し" ), "差し込み口が無くてもよい" );
	}

	Harness.BeginSuite( "CTextFormatter / 数を文字にする" );

	{
		const FTextArgument Zero = FTextArgument::FromInteger( 0 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Zero ), "0" ), "0" );

		const FTextArgument Negative = FTextArgument::FromInteger( -4567 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Negative ), "-4567" ), "負の数" );

		// 符号を反転できない値。ここを素朴に書くと桁が壊れる。
		const FTextArgument Smallest = FTextArgument::FromInteger( static_cast<i64>( -9223372036854775807LL - 1LL ) );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Smallest ), "-9223372036854775808" ),
			"i64 の最小値" );

		const FTextArgument Real = FTextArgument::FromReal( 3.14159, 2 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Real ), "3.14" ), "小数を丸める" );

		// 0.05 が ".5" にならないこと。桁の埋めを忘れると壊れる。
		const FTextArgument Small = FTextArgument::FromReal( 0.05, 2 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Small ), "0.05" ), "小数点直後の 0" );

		const FTextArgument Rounded = FTextArgument::FromReal( 1.999, 2 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Rounded ), "2.00" ), "丸めで繰り上がる" );

		const FTextArgument Whole = FTextArgument::FromReal( 12.7, 0 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), Whole ), "13" ), "小数点以下 0 桁" );

		const FTextArgument NegativeReal = FTextArgument::FromReal( -0.5, 1 );
		Harness.Check( BuiltIs( CTextFormatter::FormatOne( FStringView( "{0}" ), NegativeReal ), "-0.5" ), "負の小数" );
	}

	Harness.BeginSuite( "CTextFormatter / 崩れた書き方を残す" );

	{
		const FTextArgument One[] = { FTextArgument::FromText( FStringView( "値" ) ) };

		// 消してしまうと «なぜ出ないのか» を追えなくなるので、そのまま残す。
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{0} と {5}" ), One, 1u ), "値 と {5}" ),
			"範囲の外はそのまま残す" );
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{0} と {a}" ), One, 1u ), "値 と {a}" ),
			"数字でないものはそのまま残す" );
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "閉じない {0" ), One, 1u ), "閉じない {0" ),
			"閉じない括弧はそのまま残す" );
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{{0}}" ), One, 1u ), "{0}" ),
			"二重の括弧は括弧そのもの" );
		Harness.Check( BuiltIs( CTextFormatter::Format( FStringView( "{0}" ), nullptr, 0u ), "{0}" ),
			"値を渡さなければ何も差し込まない" );
		Harness.Check( CTextFormatter::Format( FStringView(), One, 1u ).IsEmpty(), "空の文は空のまま" );
	}

	Harness.BeginSuite( "CLocaleChangeBroadcaster / 変わったことを配る" );

	{
		CLocaleChangeBroadcaster Broadcaster;
		CCountingListener First;
		CCountingListener Second;

		Harness.Check( Broadcaster.Add( First ), "足せる" );
		Harness.Check( Broadcaster.Add( Second ), "もう 1 つ足せる" );
		Harness.Check( !Broadcaster.Add( First ), "同じ相手は二度足さない" );
		Harness.CheckEqualU64( Broadcaster.ListenerCount(), 2u, "相手の数" );

		Harness.CheckEqualU64( Broadcaster.Broadcast( ELocale::Ja ), 2u, "全員へ配る" );
		Harness.CheckEqualU64( First.CallCount, 1u, "1 度だけ呼ばれる" );
		Harness.Check( Second.LastLocale == ELocale::Ja, "変わった後の言語が届く" );

		Broadcaster.Remove( First );
		Harness.CheckEqualU64( Broadcaster.Broadcast( ELocale::En ), 1u, "外した相手には配らない" );
		Harness.CheckEqualU64( First.CallCount, 1u, "外した後は増えない" );
		Harness.CheckEqualU64( Second.CallCount, 2u, "残った相手には届く" );

		Broadcaster.Remove( First );
		Harness.CheckEqualU64( Broadcaster.ListenerCount(), 1u, "居ない相手を外しても崩れない" );

		Broadcaster.Clear();
		Harness.CheckEqualU64( Broadcaster.Broadcast( ELocale::Ja ), 0u, "全員外せる" );
	}
}

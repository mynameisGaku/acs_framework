// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSettings.h"

#include "Debug/DebugTop/Settings/DebugTopSettingsFormat.h"
#include "Debug/DebugTop/Settings/DebugTopSettingsVisitor.h"

namespace
{
	/**
	 * メニューの現在値を保管庫へ書き出す訪問者。
	 */
	/** ピン留めのキーに付ける前置き。行の値のキーとぶつからないようにする。 */
	constexpr const char* kFavoritePrefix = "@Fav/";

	/**
	 * ピン留めを保存するキーを作る。
	 *
	 * @param Key 行の保存キー。
	 * @return ピン留め用のキー。
	 */
	FString MakeFavoriteKey( const FString& Key )
	{
		FString Result( kFavoritePrefix );
		Result.Append( Key.View() );
		return Result;
	}

	/** 開閉状態のキーに付ける前置き。行の値のキーとぶつからないようにする。 */
	constexpr const char* kExpandedPrefix = "@Open/";

	/**
	 * 開閉状態を保存するキーを作る。
	 *
	 * @param Key 行の保存キー。
	 * @return 開閉用のキー。
	 */
	FString MakeExpandedKey( const FString& Key )
	{
		FString Result( kExpandedPrefix );
		Result.Append( Key.View() );
		return Result;
	}

	class CCaptureVisitor final : public IDebugTopSettingsVisitor
	{
	public:
		/**
		 * 書き出し先を指定して構築する。
		 *
		 * @param Settings 書き出し先の保管庫。
		 */
		explicit CCaptureVisitor( CDebugTopSettings& Settings ) : m_Settings( &Settings ) {}

		/** ピン留めと開閉状態を控える。 */
		void OnRow( const FString& Key, CDebugTopElement& Element ) override
		{
			// 留めていない行は書かない (保存が留めた行だけで済み、ファイルが膨らまない)。
			if ( Element.IsFavorite() ) m_Settings->SetBool( MakeFavoriteKey( Key ), true );

			// 開閉は開いた行も畳んだ行も控える。既定で開いている行を畳んだ状態も残したいので、
			// ピン留めと違って「false は書かない」ことができない。
			if ( Element.IsExpandable() && Element.HasChildren() )
			{
				m_Settings->SetBool( MakeExpandedKey( Key ), Element.IsExpanded() );
			}
		}

		/** 行の現在値を種類ごとに保管庫へ書く。 */
		void OnValue( const FString& Key, CDebugTopElement& Element ) override
		{
			switch ( Element.GetValueKind() )
			{
			case EDebugTopValueKind::Float:
			{
				f32 Value = 0.0f;
				if ( Element.TryGetFloat( Value ) ) m_Settings->SetFloat( Key, Value );
				break;
			}
			case EDebugTopValueKind::Bool:
			{
				bool bValue = false;
				if ( Element.TryGetBool( bValue ) ) m_Settings->SetBool( Key, bValue );
				break;
			}
			case EDebugTopValueKind::String:
				m_Settings->SetString( Key, Element.GetValueText() );
				break;

			default:
			{
				i32 Value = 0;
				if ( Element.TryGetInt( Value ) ) m_Settings->SetInt( Key, Value );
				break;
			}
			}
		}

	private:
		/** 書き出し先の保管庫。所有はしない。 */
		CDebugTopSettings* m_Settings;
	};

	/**
	 * 保管庫の値をメニューへ書き戻す訪問者。
	 */
	class CApplyVisitor final : public IDebugTopSettingsVisitor
	{
	public:
		/**
		 * 読み出し元を指定して構築する。
		 *
		 * @param Settings 読み出し元の保管庫。
		 */
		explicit CApplyVisitor( const CDebugTopSettings& Settings ) : m_Settings( &Settings ) {}

		/** ピン留めを書き戻す (書かれていない行は留めていない扱いにする)。 */
		void OnRow( const FString& Key, CDebugTopElement& Element ) override
		{
			bool bFavorite = false;
			m_Settings->TryGetBool( MakeFavoriteKey( Key ), bFavorite );
			Element.SetFavorite( bFavorite );

			// 開閉は控えがあるときだけ戻す。無ければページが組み立てた初期状態のままにする
			// (初めて起動したときに、開いておきたい行が畳まれてしまわないように)。
			bool bExpanded = false;
			if ( Element.IsExpandable() && m_Settings->TryGetBool( MakeExpandedKey( Key ), bExpanded ) )
			{
				Element.SetExpanded( bExpanded );
			}
		}

		/** 保管庫に同じキーがあれば行へ書き戻す。 */
		void OnValue( const FString& Key, CDebugTopElement& Element ) override
		{
			switch ( Element.GetValueKind() )
			{
			case EDebugTopValueKind::Float:
			{
				f32 Value = 0.0f;
				if ( m_Settings->TryGetFloat( Key, Value ) ) Element.TrySetFloat( Value );
				break;
			}
			case EDebugTopValueKind::Bool:
			{
				bool bValue = false;
				if ( m_Settings->TryGetBool( Key, bValue ) ) Element.TrySetBool( bValue );
				break;
			}
			case EDebugTopValueKind::String:
			{
				// 文字列の行は打ち込みと同じ経路で書き戻す。
				FString Value;
				if ( m_Settings->TryGetString( Key, Value ) ) Element.CommitEditText( Value );
				break;
			}
			default:
			{
				i32 Value = 0;
				if ( m_Settings->TryGetInt( Key, Value ) ) Element.TrySetInt( Value );
				break;
			}
			}
		}

	private:
		/** 読み出し元の保管庫。所有はしない。 */
		const CDebugTopSettings* m_Settings;
	};

	/**
	 * 置き場所が無ければ作る。
	 *
	 * @details 1 段だけ作る (深い階層をまとめて作りたい場合は呼び出し側で用意すること)。
	 * @param Directory 置き場所 (空なら何もしない)。
	 */
	void EnsureDirectory( const FString& Directory )
	{
		if ( Directory.IsEmpty() ) return;

		TArray<wchar_t> WideDirectory;
		if ( !DebugTopToWidePath( Directory, WideDirectory ) ) return;
		if ( CFileSystem::DirectoryExists( WideDirectory.GetData() ) ) return;

		const auto Result = CFileSystem::CreateDirectory( WideDirectory.GetData() );
		if ( Result.IsErr() )
		{
			ACS_LOG_WARN( "CDebugTopSettings: 置き場所を作れなかった (%s)", Result.Error().message );
		}
	}
}


// GameInstance スコープへ登録する。シーンを切り替えても実体が残るので、メニューで設定した値を
// 次のレベルがそのまま受け取れる。取得は GetSubsystem<CDebugTopSettings>()。
ACS_REGISTER_SUBSYSTEM( CDebugTopSettings, ESubsystemScope::GameInstance )


FDebugTopSetting* CDebugTopSettings::Find( const FString& Key ) noexcept
{
	for ( usize Index = 0; Index < m_Settings.Num(); ++Index )
	{
		if ( m_Settings[Index].Key == Key ) return &m_Settings[Index];
	}
	return nullptr;
}

const FDebugTopSetting* CDebugTopSettings::Find( const FString& Key ) const noexcept
{
	for ( usize Index = 0; Index < m_Settings.Num(); ++Index )
	{
		if ( m_Settings[Index].Key == Key ) return &m_Settings[Index];
	}
	return nullptr;
}

FDebugTopSetting& CDebugTopSettings::FindOrAdd( const FString& Key )
{
	if ( FDebugTopSetting* const Found = Find( Key ) ) return *Found;

	FDebugTopSetting Setting;
	Setting.Key = Key;
	m_Settings.Add( Move( Setting ) );
	return m_Settings[m_Settings.Num() - 1];
}

bool CDebugTopSettings::TryGetInt( const FString& Key, i32& OutValue ) const noexcept
{
	const FDebugTopSetting* const Setting = Find( Key );
	if ( Setting == nullptr || Setting->Kind != EDebugTopSettingKind::Int ) return false;

	OutValue = Setting->IntValue;
	return true;
}

bool CDebugTopSettings::TryGetFloat( const FString& Key, f32& OutValue ) const noexcept
{
	const FDebugTopSetting* const Setting = Find( Key );
	if ( Setting == nullptr || Setting->Kind != EDebugTopSettingKind::Float ) return false;

	OutValue = Setting->FloatValue;
	return true;
}

bool CDebugTopSettings::TryGetBool( const FString& Key, bool& bOutValue ) const noexcept
{
	const FDebugTopSetting* const Setting = Find( Key );
	if ( Setting == nullptr || Setting->Kind != EDebugTopSettingKind::Bool ) return false;

	bOutValue = Setting->bBoolValue;
	return true;
}

bool CDebugTopSettings::TryGetString( const FString& Key, FString& OutValue ) const
{
	const FDebugTopSetting* const Setting = Find( Key );
	if ( Setting == nullptr || Setting->Kind != EDebugTopSettingKind::String ) return false;

	OutValue = Setting->StringValue;
	return true;
}

i32 CDebugTopSettings::GetInt( const FString& Key, i32 DefaultValue ) const noexcept
{
	i32 Value = 0;
	return TryGetInt( Key, Value ) ? Value : DefaultValue;
}

f32 CDebugTopSettings::GetFloat( const FString& Key, f32 DefaultValue ) const noexcept
{
	f32 Value = 0.0f;
	return TryGetFloat( Key, Value ) ? Value : DefaultValue;
}

bool CDebugTopSettings::GetBool( const FString& Key, bool bDefaultValue ) const noexcept
{
	bool bValue = false;
	return TryGetBool( Key, bValue ) ? bValue : bDefaultValue;
}

void CDebugTopSettings::SetInt( const FString& Key, i32 Value )
{
	FDebugTopSetting& Setting = FindOrAdd( Key );
	Setting.Kind = EDebugTopSettingKind::Int;
	Setting.IntValue = Value;
}

void CDebugTopSettings::SetFloat( const FString& Key, f32 Value )
{
	FDebugTopSetting& Setting = FindOrAdd( Key );
	Setting.Kind = EDebugTopSettingKind::Float;
	Setting.FloatValue = Value;
}

void CDebugTopSettings::SetBool( const FString& Key, bool bValue )
{
	FDebugTopSetting& Setting = FindOrAdd( Key );
	Setting.Kind = EDebugTopSettingKind::Bool;
	Setting.bBoolValue = bValue;
}

void CDebugTopSettings::SetString( const FString& Key, const FString& Value )
{
	FDebugTopSetting& Setting = FindOrAdd( Key );
	Setting.Kind = EDebugTopSettingKind::String;
	Setting.StringValue = Value;

	// テキスト形式は 1 行 1 設定なので、改行はそのまま持たせない。
	for ( usize Index = 0; Index < Setting.StringValue.Size(); ++Index )
	{
		char& Character = Setting.StringValue[Index];
		if ( Character == '\n' || Character == '\r' ) Character = ' ';
	}
}

bool CDebugTopSettings::Remove( const FString& Key ) noexcept
{
	for ( usize Index = 0; Index < m_Settings.Num(); ++Index )
	{
		if ( !( m_Settings[Index].Key == Key ) ) continue;

		m_Settings.RemoveAt( Index );
		return true;
	}
	return false;
}

void CDebugTopSettings::Clear() noexcept
{
	m_Settings.Reset();
}

void CDebugTopSettings::CaptureFrom( const ADebugTopHUD& HUD )
{
	CCaptureVisitor Visitor( *this );
	DebugTopVisitSettings( HUD, Visitor );
}

void CDebugTopSettings::ApplyTo( ADebugTopHUD& HUD ) const
{
	CApplyVisitor Visitor( *this );
	DebugTopVisitSettings( HUD, Visitor );
}

bool CDebugTopSettings::Save() const
{
	EnsureDirectory( m_Path.GetDirectory() );

	const FString Path = m_Path.Build();
	if ( !SaveAs( Path, m_Path.GetFormat() ) ) return false;

	// どこへ書いたかが分からないと探しに行けないので、絶対パスで出す。
	const FString Absolute = m_Path.BuildAbsolute();
	const FString FormatName = DebugTopSettingsFormatName( m_Path.GetFormat() );
	ACS_LOG_INFO( "CDebugTopSettings: %zu 件を %s 形式で保存した -> %s", m_Settings.Num(), FormatName.Data(), Absolute.Data() );
	return true;
}

bool CDebugTopSettings::Load()
{
	const FString Path = m_Path.Build();
	if ( !LoadFrom( Path ) ) return false;

	const FString Absolute = m_Path.BuildAbsolute();
	ACS_LOG_INFO( "CDebugTopSettings: %zu 件を読み込んだ <- %s", m_Settings.Num(), Absolute.Data() );
	return true;
}

bool CDebugTopSettings::SaveAs( const FString& Path, EDebugTopSettingsFormat Format ) const
{
	TArray<wchar_t> WidePath;
	if ( !DebugTopToWidePath( Path, WidePath ) ) return false;

	TArray<byte> Bytes;
	if ( !DebugTopSerializeSettings( m_Settings, Format, Bytes ) ) return false;

	// 書き込み途中で落ちても前の内容を壊さないよう、原子的に差し替える。
	const auto Result = CFileSystem::WriteAllBytesAtomic( WidePath.GetData(), Bytes.GetData(), Bytes.Num() );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CDebugTopSettings: 設定を保存できなかった (%s)", Result.Error().message );
		return false;
	}
	return true;
}

bool CDebugTopSettings::LoadFrom( const FString& Path )
{
	TArray<wchar_t> WidePath;
	if ( !DebugTopToWidePath( Path, WidePath ) ) return false;

	// 未保存のうちは毎回失敗するので、読めないこと自体は警告にしない。
	auto Result = CFileSystem::ReadAllBytes( WidePath.GetData() );
	if ( Result.IsErr() ) return false;

	const TArray<byte>& Bytes = Result.Value();
	if ( Bytes.IsEmpty() ) return false;

	return DebugTopDeserializeSettings( Bytes.GetData(), Bytes.Num(), m_Settings );
}

#include "DebugTopFileDialog.h"

#include <windows.h>
#include <shobjidl.h>

namespace
{
	/**
	 * UTF-8 を UTF-16 へ変換する。
	 *
	 * @param Utf8 変換元。
	 * @param OutWide 変換先 (NUL 終端まで含めて積む)。
	 * @return 変換できたら true。
	 */
	bool ToWide( const FString& Utf8, TArray<wchar_t>& OutWide )
	{
		OutWide.Reset();
		if ( Utf8.IsEmpty() )
		{
			OutWide.Add( L'\0' );
			return true;
		}

		const int Length = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), static_cast<int>( Utf8.Size() ), nullptr, 0 );
		if ( Length <= 0 ) return false;

		OutWide.SetNum( static_cast<usize>( Length ) + 1 );
		::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), static_cast<int>( Utf8.Size() ), OutWide.GetData(), Length );
		OutWide[static_cast<usize>( Length )] = L'\0';
		return true;
	}

	/**
	 * UTF-16 を UTF-8 へ変換する。
	 *
	 * @param Wide 変換元 (NUL 終端)。
	 * @param OutUtf8 変換先。
	 * @return 変換できたら true。
	 */
	bool ToUtf8( const wchar_t* Wide, FString& OutUtf8 )
	{
		if ( Wide == nullptr ) return false;

		const int Length = ::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, nullptr, 0, nullptr, nullptr );
		if ( Length <= 1 ) return false;

		TArray<char> Buffer;
		Buffer.SetNum( static_cast<usize>( Length ) );
		::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, Buffer.GetData(), Length, nullptr, nullptr );

		// 末尾の NUL は FString へ含めない。
		OutUtf8 = FString( FStringView( Buffer.GetData(), static_cast<usize>( Length ) - 1 ) );
		return true;
	}
}


bool DebugTopPickPath( EDebugTopPickKind Kind, const FString& Title, const FString& Initial, FString& OutPath )
{
	// このスレッドが既に COM を初期化していることもあるので、その場合も進める。
	const HRESULT InitResult = ::CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
	const bool bNeedsUninit = SUCCEEDED( InitResult );

	bool bPicked = false;
	IFileOpenDialog* Dialog = nullptr;
	if ( SUCCEEDED( ::CoCreateInstance( CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &Dialog ) ) ) && Dialog != nullptr )
	{
		DWORD Options = 0;
		if ( SUCCEEDED( Dialog->GetOptions( &Options ) ) )
		{
			// フォルダを選ばせるときだけ、ファイルではなくフォルダを拾う設定にする。
			if ( Kind == EDebugTopPickKind::Folder ) Options |= FOS_PICKFOLDERS;
			Dialog->SetOptions( Options | FOS_FORCEFILESYSTEM );
		}

		TArray<wchar_t> WideTitle;
		if ( ToWide( Title, WideTitle ) ) Dialog->SetTitle( WideTitle.GetData() );

		// 最初に開く場所。指定が無い / 見つからない場合は OS 任せにする。
		TArray<wchar_t> WideInitial;
		if ( !Initial.IsEmpty() && ToWide( Initial, WideInitial ) )
		{
			IShellItem* Folder = nullptr;
			if ( SUCCEEDED( ::SHCreateItemFromParsingName( WideInitial.GetData(), nullptr, IID_PPV_ARGS( &Folder ) ) ) && Folder != nullptr )
			{
				Dialog->SetFolder( Folder );
				Folder->Release();
			}
		}

		if ( SUCCEEDED( Dialog->Show( nullptr ) ) )
		{
			IShellItem* Item = nullptr;
			if ( SUCCEEDED( Dialog->GetResult( &Item ) ) && Item != nullptr )
			{
				PWSTR Path = nullptr;
				if ( SUCCEEDED( Item->GetDisplayName( SIGDN_FILESYSPATH, &Path ) ) )
				{
					bPicked = ToUtf8( Path, OutPath );
					::CoTaskMemFree( Path );
				}
				Item->Release();
			}
		}
		Dialog->Release();
	}

	if ( bNeedsUninit ) ::CoUninitialize();
	return bPicked;
}

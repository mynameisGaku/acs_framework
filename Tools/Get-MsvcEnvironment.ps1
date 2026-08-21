# SPDX-License-Identifier: Apache-2.0

$candidates = [System.Collections.Generic.List[string]]::new()
if (-not [string]::IsNullOrWhiteSpace($env:VSINSTALLDIR)) {
    $candidates.Add($env:VSINSTALLDIR.TrimEnd('\', '/'))
}

foreach ($version in @('18', '17')) {
    foreach ($edition in @('Community', 'Professional', 'Enterprise', 'BuildTools')) {
        $candidates.Add("C:\Program Files\Microsoft Visual Studio\$version\$edition")
    }
}

foreach ($installPath in $candidates | Select-Object -Unique) {
    $devCommandPath = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
    if (-not (Test-Path -LiteralPath $devCommandPath -PathType Leaf)) { continue }

    $command = '"{0}" -arch=amd64 -host_arch=amd64 -no_logo >nul && set' -f $devCommandPath
    $environmentOutput = & $env:ComSpec /d /s /c $command
    if ($LASTEXITCODE -eq 0) {
        [string]::Join("`n", @($environmentOutput)) -split '\r?\n' | Where-Object { $_.Length -gt 0 }
        return
    }
}

throw 'x64 MSVC compiler environment could not be read.'

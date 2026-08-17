# SPDX-License-Identifier: Apache-2.0
#
# Build and run selected ACS engine tests without configuring the whole CMake suite.
#
# When this framework needs something added to ACS itself, the change lands in the ACS
# worktree. Compiling it is not enough: a test that compiles can still assert the wrong
# thing. This script links the chosen acs/tests/*.cpp against the deployed distribution
# and actually runs them, which takes seconds instead of a full engine configure.
#
# The authoritative path is still ACS's own CMake suite (generate.ps1 -Tests). This is a
# shortcut for the edit-run loop, not a replacement.
#
# Usage:
#   .\Tools\RunAcsTests.ps1 camera3d_tests light_component3d_tests
#   .\Tools\RunAcsTests.ps1 -All
#   .\Tools\RunAcsTests.ps1 -Worktree C:\dev\acs_dev -AcsDistRoot C:\acs_dev camera3d_tests
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Position = 0, ValueFromRemainingArguments = $true)]
    [string[]]$Tests,
    [string]$Worktree    = 'C:\dev\acs_dev',
    [string]$AcsDistRoot = $env:ACS_DIST_ROOT,
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$All
)
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($AcsDistRoot)) { $AcsDistRoot = 'C:\acs_dev' }

$src      = Join-Path $Worktree 'acs\src'
$testDir  = Join-Path $Worktree 'acs\tests'
$libDir   = Join-Path $AcsDistRoot "lib\x64\$Configuration"

foreach ($required in @($src, $testDir, $libDir)) {
    if (-not (Test-Path $required)) { throw "not found: $required" }
}

# Pick the test translation units to build.
if ($All) {
    $files = Get-ChildItem $testDir -Filter '*_tests.cpp' | ForEach-Object { $_.FullName }
} else {
    if (-not $Tests -or $Tests.Count -eq 0) {
        throw "Name at least one test (without the .cpp), or pass -All."
    }
    $files = foreach ($name in $Tests) {
        $leaf = if ($name.EndsWith('.cpp')) { $name } else { "$name.cpp" }
        $path = Join-Path $testDir $leaf
        if (-not (Test-Path $path)) { throw "test not found: $path" }
        $path
    }
}

$out = Join-Path $env:TEMP ("acs-tests-" + $Configuration)
New-Item -ItemType Directory -Force -Path $out | Out-Null

# The ACS test framework registers cases at static-init time and RunAll executes them.
$mainPath = Join-Path $out 'main.cpp'
Set-Content -Path $mainPath -Encoding utf8 -Value @'
#include "test/Test.h"
int main() { return acs::test::RunAll(); }
'@

# ACS is built with exceptions and RTTI disabled. A consumer TU must match or the
# distribution header refuses to compile.
$flags = @('/nologo', '/std:c++20', '/utf-8', '/permissive-', '/Zc:__cplusplus', '/Zc:preprocessor',
           '/EHs-c-', '/GR-', '/D_HAS_EXCEPTIONS=0', '/DPLATFORM_WIN32=1', '/DNOMINMAX')
$flags += if ($Configuration -eq 'Debug') { '/MDd' } else { '/MD' }

# The distribution ships Diligent as separate libraries next to acs.lib, so link the
# whole directory plus the Windows libraries those objects pull in.
$libs = (Get-ChildItem $libDir -Filter *.lib | ForEach-Object { $_.Name }) +
        @('d3d12.lib', 'dxgi.lib', 'dxguid.lib', 'd3dcompiler.lib', 'comdlg32.lib',
          'advapi32.lib', 'user32.lib', 'shell32.lib', 'psapi.lib', 'bcrypt.lib',
          'ole32.lib', 'oleaut32.lib')

$exe = Join-Path $out 'AcsTests.exe'
$clArgs = $flags + @('/I', $src, "/Fe$exe", "/Fo$out\") + @($mainPath, (Join-Path $src 'test\Test.cpp')) + $files +
          @('/link', "/LIBPATH:$libDir", '/SUBSYSTEM:CONSOLE') + $libs

Write-Host ("==> building " + @($files).Count + " test file(s)") -ForegroundColor Cyan
$buildLog = & cl @clArgs 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) {
    Write-Host $buildLog
    throw "build failed"
}

Write-Host '==> running' -ForegroundColor Cyan
& $exe
exit $LASTEXITCODE

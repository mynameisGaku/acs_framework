param(
    # Target .vcxproj. Defaults to acs_framework.vcxproj next to the Tools folder.
    [string]$ProjectPath,
    # Folder trees that define the filter layout. Empty folders are kept.
    # All four are listed so a plain invocation is the canonical one: if -Check
    # and the regeneration disagree on this, -Check reports stale forever.
    [string[]]$FolderRoots = @('Source', 'Docs', 'Tools', 'Assets'),
    # Only rewrite the .filters: leave the .vcxproj item list alone and just
    # report the files that are on disk without being part of the project.
    [switch]$FiltersOnly,
    # Report what would be written without touching anything. Exit code 2 when
    # this invocation would have changed a file.
    [switch]$Check
)

# Rebuilds <project>.vcxproj.filters so the Solution Explorer tree mirrors the
# folder layout on disk: every directory under -FolderRoots becomes a filter and
# every project item is filed under the directory it actually lives in.
# Source files found on disk are added to the .vcxproj and entries whose file is
# gone are dropped, so that the tree really does match the folder.
# Comments and messages stay ASCII so console output never mojibakes.

$ErrorActionPreference = 'Stop'

$msbuildNamespace = 'http://schemas.microsoft.com/developer/msbuild/2003'
$itemTypeOrder = @(
    'ClCompile', 'ClInclude', 'None', 'ResourceCompile',
    'Natvis', 'CustomBuild', 'Image', 'Text', 'Xml', 'Manifest')
$compileExtensions = @('.c', '.cc', '.cpp', '.cxx')
$includeExtensions = @('.h', '.hh', '.hpp', '.hxx', '.inl', '.ipp')
# The project files in this repository are UTF-8 without BOM and CRLF.
$newLine = "`r`n"
$fileEncoding = New-Object System.Text.UTF8Encoding($false)

function ConvertTo-XmlAttributeText([string]$Value) {
    return $Value.Replace('&', '&amp;').Replace('<', '&lt;').
        Replace('>', '&gt;').Replace('"', '&quot;')
}

# Path relative to the project directory, or $null when it escapes the tree.
function Resolve-ProjectRelativePath([string]$ProjectDirectory, [string]$Path) {
    $prefix = $ProjectDirectory.TrimEnd('\') + '\'
    $full = [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($ProjectDirectory, $Path)))
    if (-not $full.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $null
    }
    return $full.Substring($prefix.Length)
}

# Filter GUIDs must survive regeneration, so derive them from the filter path.
function New-StableFilterGuid([string]$FilterPath) {
    $algorithm = [System.Security.Cryptography.SHA256]::Create()
    try {
        $hash = $algorithm.ComputeHash(
            [System.Text.Encoding]::UTF8.GetBytes($FilterPath.ToLowerInvariant()))
    } finally {
        $algorithm.Dispose()
    }
    $guidBytes = New-Object byte[] 16
    [System.Array]::Copy($hash, $guidBytes, 16)
    return (New-Object System.Guid(, $guidBytes)).ToString('B')
}

function Sort-PathList([string[]]$Values) {
    $sorted = [string[]]$Values
    [System.Array]::Sort($sorted, [System.StringComparer]::OrdinalIgnoreCase)
    return $sorted
}

function Get-ProjectItemNodes($Document, $NamespaceManager, [string[]]$Types) {
    $nodes = New-Object System.Collections.ArrayList
    foreach ($type in $Types) {
        foreach ($node in $Document.SelectNodes(
                "/msb:Project/msb:ItemGroup/msb:$type", $NamespaceManager)) {
            $include = $node.GetAttribute('Include')
            if ([string]::IsNullOrWhiteSpace($include)) { continue }
            # Wildcards and property references are not fixed paths.
            if ($include.Contains('*') -or $include.Contains('$(')) { continue }
            $null = $nodes.Add([pscustomobject]@{
                Type = $type
                Include = $include
                Node = $node
            })
        }
    }
    return $nodes
}

function Test-BytesEqual($Left, $Right) {
    if ($null -eq $Left -or $null -eq $Right) { return $false }
    if ($Left.Length -ne $Right.Length) { return $false }
    for ($index = 0; $index -lt $Left.Length; $index++) {
        if ($Left[$index] -ne $Right[$index]) { return $false }
    }
    return $true
}

if (-not $ProjectPath) {
    $ProjectPath = Join-Path (Split-Path -Parent $PSScriptRoot) 'acs_framework.vcxproj'
}
$ProjectPath = [System.IO.Path]::GetFullPath($ProjectPath)
if (-not (Test-Path -LiteralPath $ProjectPath -PathType Leaf)) {
    throw "project not found: $ProjectPath"
}
$projectDirectory = [System.IO.Path]::GetDirectoryName($ProjectPath)
$filtersPath = $ProjectPath + '.filters'

$document = New-Object System.Xml.XmlDocument
# Keep the original indentation of the .vcxproj when -SyncProjectItems writes it.
$document.PreserveWhitespace = $true
$document.Load($ProjectPath)
$namespaceManager = New-Object System.Xml.XmlNamespaceManager($document.NameTable)
$namespaceManager.AddNamespace('msb', $msbuildNamespace)

# ---- scan the folder roots -------------------------------------------------
$directoryFilters = New-Object System.Collections.Generic.HashSet[string](
    [System.StringComparer]::OrdinalIgnoreCase)
$diskFiles = New-Object System.Collections.ArrayList
foreach ($root in $FolderRoots) {
    $rootFull = [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($projectDirectory, $root)))
    if (-not (Test-Path -LiteralPath $rootFull -PathType Container)) {
        Write-Host "warning: folder root not found: $root"
        continue
    }
    $rootRelative = Resolve-ProjectRelativePath $projectDirectory $rootFull
    if ($rootRelative) { $null = $directoryFilters.Add($rootRelative) }
    foreach ($directory in Get-ChildItem -LiteralPath $rootFull -Recurse -Directory) {
        $relative = Resolve-ProjectRelativePath $projectDirectory $directory.FullName
        if ($relative) { $null = $directoryFilters.Add($relative) }
    }
    foreach ($file in Get-ChildItem -LiteralPath $rootFull -Recurse -File) {
        $null = $diskFiles.Add($file.FullName)
    }
}

# ---- bring the .vcxproj item list back in line with the disk ---------------
$itemsAdded = @()
$itemsRemoved = @()
$projectWouldChange = $false
if (-not $FiltersOnly) {
    $sourceNodes = Get-ProjectItemNodes $document $namespaceManager @('ClCompile', 'ClInclude')
    $knownPaths = New-Object System.Collections.Generic.HashSet[string](
        [System.StringComparer]::OrdinalIgnoreCase)
    foreach ($item in $sourceNodes) {
        $null = $knownPaths.Add(
            [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($projectDirectory, $item.Include))))
    }

    foreach ($diskFile in Sort-PathList ([string[]]$diskFiles)) {
        if ($knownPaths.Contains($diskFile)) { continue }
        $extension = [System.IO.Path]::GetExtension($diskFile).ToLowerInvariant()
        $type = $null
        if ($compileExtensions -contains $extension) { $type = 'ClCompile' }
        elseif ($includeExtensions -contains $extension) { $type = 'ClInclude' }
        if (-not $type) { continue }

        $relative = Resolve-ProjectRelativePath $projectDirectory $diskFile
        if (-not $relative) { continue }

        # Append to the last ItemGroup that already holds this item type, so the
        # existing grouping in the .vcxproj is preserved.
        $existingOfType = $document.SelectNodes(
            "/msb:Project/msb:ItemGroup/msb:$type", $namespaceManager)
        if ($existingOfType.Count -gt 0) {
            $group = $existingOfType.Item($existingOfType.Count - 1).ParentNode
        } else {
            $group = $document.CreateElement('ItemGroup', $msbuildNamespace)
            $anchor = $document.SelectSingleNode(
                "/msb:Project/msb:Import[contains(@Project,'Microsoft.Cpp.targets')]",
                $namespaceManager)
            # The whitespace that already indents the anchor becomes the indent
            # of the new group, so a fresh one is added back before the anchor.
            $null = $document.DocumentElement.InsertBefore($group, $anchor)
            $null = $document.DocumentElement.InsertBefore(
                $document.CreateWhitespace($newLine + '  '), $anchor)
            $null = $group.AppendChild($document.CreateWhitespace($newLine + '  '))
        }

        $element = $document.CreateElement($type, $msbuildNamespace)
        $element.SetAttribute('Include', $relative)
        $tail = $group.LastChild
        if ($null -ne $tail -and $tail.NodeType -eq [System.Xml.XmlNodeType]::Whitespace) {
            $null = $group.InsertBefore($element, $tail)
        } else {
            $null = $group.AppendChild($element)
        }
        $null = $group.InsertBefore($document.CreateWhitespace($newLine + '    '), $element)
        $itemsAdded += $relative
    }

    foreach ($item in $sourceNodes) {
        $full = [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($projectDirectory, $item.Include)))
        if (Test-Path -LiteralPath $full -PathType Leaf) { continue }
        $node = $item.Node
        $previous = $node.PreviousSibling
        $null = $node.ParentNode.RemoveChild($node)
        if ($null -ne $previous -and
            $previous.NodeType -eq [System.Xml.XmlNodeType]::Whitespace) {
            $null = $previous.ParentNode.RemoveChild($previous)
        }
        $itemsRemoved += $item.Include
    }

    if ($itemsAdded.Count -gt 0 -or $itemsRemoved.Count -gt 0) {
        $writerSettings = New-Object System.Xml.XmlWriterSettings
        $writerSettings.Encoding = $fileEncoding
        $writerSettings.Indent = $false
        $writerSettings.NewLineHandling = [System.Xml.NewLineHandling]::Replace
        $writerSettings.NewLineChars = $newLine
        $stream = New-Object System.IO.MemoryStream
        $writer = [System.Xml.XmlWriter]::Create($stream, $writerSettings)
        try {
            $document.Save($writer)
        } finally {
            $writer.Dispose()
        }
        $projectBytes = $stream.ToArray()
        $stream.Dispose()
        $projectWouldChange = $true
        $prefix = ''
        if ($Check) {
            $prefix = 'would_'
        } else {
            [System.IO.File]::WriteAllBytes($ProjectPath, $projectBytes)
        }
        foreach ($added in $itemsAdded) { Write-Host "${prefix}project_item_added: $added" }
        foreach ($removed in $itemsRemoved) { Write-Host "${prefix}project_item_removed: $removed" }
    }
}

# ---- collect the items that need a filter ----------------------------------
$items = Get-ProjectItemNodes $document $namespaceManager $itemTypeOrder
$itemFilters = @{}
foreach ($item in $items) {
    $relative = Resolve-ProjectRelativePath $projectDirectory $item.Include
    if (-not $relative) { continue }
    $directory = [System.IO.Path]::GetDirectoryName($relative)
    if ([string]::IsNullOrEmpty($directory)) { continue }
    $itemFilters[$item.Include] = $directory
    # Every ancestor needs a filter element as well.
    $current = $directory
    while (-not [string]::IsNullOrEmpty($current)) {
        $null = $directoryFilters.Add($current)
        $current = [System.IO.Path]::GetDirectoryName($current)
    }
}

# ---- report drift between the project and the disk -------------------------
$knownItemPaths = New-Object System.Collections.Generic.HashSet[string](
    [System.StringComparer]::OrdinalIgnoreCase)
foreach ($item in $items) {
    $null = $knownItemPaths.Add(
        [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($projectDirectory, $item.Include))))
}
$driftCount = 0
foreach ($diskFile in Sort-PathList ([string[]]$diskFiles)) {
    $extension = [System.IO.Path]::GetExtension($diskFile).ToLowerInvariant()
    if (-not (($compileExtensions -contains $extension) -or
              ($includeExtensions -contains $extension))) { continue }
    if ($knownItemPaths.Contains($diskFile)) { continue }
    $driftCount++
    Write-Host ("warning: on disk but not in the project: " +
        (Resolve-ProjectRelativePath $projectDirectory $diskFile))
}
foreach ($item in $items) {
    $full = [System.IO.Path]::GetFullPath(([System.IO.Path]::Combine($projectDirectory, $item.Include)))
    if (Test-Path -LiteralPath $full -PathType Leaf) { continue }
    $driftCount++
    Write-Host "warning: in the project but missing on disk: $($item.Include)"
}
if ($driftCount -gt 0 -and $FiltersOnly) {
    Write-Host 'hint: rerun without -FiltersOnly to fix the item list'
}

# ---- reuse the GUIDs the existing .filters already assigned ----------------
$existingGuids = @{}
$toolsVersion = '4.0'
if (Test-Path -LiteralPath $filtersPath -PathType Leaf) {
    $existingDocument = New-Object System.Xml.XmlDocument
    $existingDocument.Load($filtersPath)
    $existingNamespaces = New-Object System.Xml.XmlNamespaceManager($existingDocument.NameTable)
    $existingNamespaces.AddNamespace('msb', $msbuildNamespace)
    if ($existingDocument.DocumentElement.HasAttribute('ToolsVersion')) {
        $toolsVersion = $existingDocument.DocumentElement.GetAttribute('ToolsVersion')
    }
    foreach ($node in $existingDocument.SelectNodes(
            '/msb:Project/msb:ItemGroup/msb:Filter', $existingNamespaces)) {
        $include = $node.GetAttribute('Include')
        $identifier = $node.SelectSingleNode('msb:UniqueIdentifier', $existingNamespaces)
        if ($include -and $null -ne $identifier) {
            $existingGuids[$include] = $identifier.InnerText
        }
    }
}

# ---- build the .filters document -------------------------------------------
$builder = New-Object System.Text.StringBuilder
$null = $builder.Append('<?xml version="1.0" encoding="utf-8"?>').Append($newLine)
$null = $builder.Append(
    "<Project ToolsVersion=""$toolsVersion"" xmlns=""$msbuildNamespace"">").Append($newLine)

$sortedFilters = Sort-PathList ([string[]]@($directoryFilters))
if ($sortedFilters.Count -gt 0) {
    $null = $builder.Append('  <ItemGroup>').Append($newLine)
    foreach ($filter in $sortedFilters) {
        $guid = $existingGuids[$filter]
        if (-not $guid) { $guid = New-StableFilterGuid $filter }
        $null = $builder.Append(
            '    <Filter Include="' + (ConvertTo-XmlAttributeText $filter) + '">').Append($newLine)
        $null = $builder.Append(
            "      <UniqueIdentifier>$guid</UniqueIdentifier>").Append($newLine)
        $null = $builder.Append('    </Filter>').Append($newLine)
    }
    $null = $builder.Append('  </ItemGroup>').Append($newLine)
}

$filteredItemCount = 0
foreach ($type in $itemTypeOrder) {
    $ofType = @($items | Where-Object { $_.Type -eq $type -and $itemFilters.ContainsKey($_.Include) })
    if ($ofType.Count -eq 0) { continue }
    $null = $builder.Append('  <ItemGroup>').Append($newLine)
    foreach ($include in Sort-PathList ([string[]]($ofType | ForEach-Object { $_.Include }))) {
        $null = $builder.Append(
            '    <' + $type + ' Include="' + (ConvertTo-XmlAttributeText $include) + '">').Append($newLine)
        $null = $builder.Append(
            '      <Filter>' + (ConvertTo-XmlAttributeText $itemFilters[$include]) + '</Filter>').Append($newLine)
        $null = $builder.Append('    </' + $type + '>').Append($newLine)
        $filteredItemCount++
    }
    $null = $builder.Append('  </ItemGroup>').Append($newLine)
}
# VS writes no trailing newline after the root element.
$null = $builder.Append('</Project>')

$filtersBytes = $fileEncoding.GetBytes($builder.ToString())
$currentBytes = $null
if (Test-Path -LiteralPath $filtersPath -PathType Leaf) {
    $currentBytes = [System.IO.File]::ReadAllBytes($filtersPath)
}
$filtersName = [System.IO.Path]::GetFileName($filtersPath)

if (Test-BytesEqual $currentBytes $filtersBytes) {
    Write-Host "filters_up_to_date: $filtersName (filters=$($sortedFilters.Count) items=$filteredItemCount)"
    if ($Check -and $projectWouldChange) { exit 2 }
    exit 0
}

if ($Check) {
    Write-Host "filters_out_of_date: $filtersName"
    exit 2
}

[System.IO.File]::WriteAllBytes($filtersPath, $filtersBytes)
Write-Host "filters_updated: $filtersName (filters=$($sortedFilters.Count) items=$filteredItemCount)"
exit 0

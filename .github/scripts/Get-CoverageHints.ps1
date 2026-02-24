#Requires -Version 5.1
<#
.SYNOPSIS
    Generates actionable coverage failure hints from Cobertura XML.
.DESCRIPTION
    Reads CoverageReports\coverage.xml and identifies production files below the
    configured threshold. Writes HINT_TEXT to GITHUB_ENV for use in PR comments.
    Skips test files (Private\Tests\). No output if coverage passes or no files found.
#>

[CmdletBinding()]
param(
    [string]$CoberturaXmlPath = 'CoverageReports\coverage.xml',
    [int]$Threshold = 70,
    [int]$MaxFiles = 5
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $CoberturaXmlPath)) {
    Write-Host "Hints: coverage.xml not found, skipping hint generation"
    return
}

[xml]$cov    = Get-Content $CoberturaXmlPath -Encoding UTF8
$lineRate    = [double]$cov.coverage.GetAttribute('line-rate')
$coveragePct = [math]::Round($lineRate * 100, 2)

if ($coveragePct -ge $Threshold) {
    if ($env:GITHUB_ENV) {
        "HINT_TEXT=" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    }
    return
}

$lowFiles = @()
foreach ($class in $cov.coverage.packages.package.classes.class) {
    $fileRate  = [double]$class.GetAttribute('line-rate')
    $filename  = $class.GetAttribute('filename')
    if ($filename -notmatch '\\Tests\\') {
        $filePct  = [math]::Round($fileRate * 100, 0)
        if ($filePct -lt $Threshold) {
            $shortName = Split-Path $filename -Leaf
            $lowFiles += "  - ``$shortName``: $filePct%"
        }
    }
}

if ($lowFiles.Count -gt 0) {
    $shown     = $lowFiles | Select-Object -First $MaxFiles
    $hintLines = @('**Files below threshold:**') + $shown
    if ($lowFiles.Count -gt $MaxFiles) {
        $hintLines += "  - ...and $($lowFiles.Count - $MaxFiles) more"
    }
    $hintText = $hintLines -join "`n"
    if ($env:GITHUB_ENV) {
        $delim = "EOF_HINT_$(Get-Random)"
        "HINT_TEXT<<$delim", $hintText, $delim | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    }
    Write-Host "Hints generated for $($lowFiles.Count) files below threshold"
} else {
    if ($env:GITHUB_ENV) {
        "HINT_TEXT=" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    }
}

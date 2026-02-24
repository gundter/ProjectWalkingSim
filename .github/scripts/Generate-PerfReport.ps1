# Generate-PerfReport.ps1
# Performance report generation pipeline for Gauntlet E2E test runs.
#
# Pipeline stages:
#   1. Collect CSV files from packaged build's Saved/Profiling/CSV/ directory
#   2. Run PerfreportTool per collected CSV to produce per-node HTML reports
#   3. Parse summary stats and check configurable performance thresholds
#   4. Generate master performance-report.html (dashboard + iframes + crash section)
#   5. Output threshold violation count and return appropriate exit code
#
# Usage:
#   .\Generate-PerfReport.ps1 -WorkspaceDir "$env:GITHUB_WORKSPACE" `
#       -UE5Root "H:\UE5-Source\UnrealEngine" `
#       -ReportDir "GauntletResults" `
#       -CommitSha "abc1234" `
#       -RunNumber "42"
#
# Threshold enforcement:
#   By default, threshold violations emit GitHub ::warning:: annotations.
#   Pass -FailOnThreshold to promote violations to ::error:: and exit 1.
#
# CSV filename convention (from Plan 16-01):
#   BootTest.csv           -- from UClaudeBootTestController BeginCapture
#   GameplayTest.csv       -- from UClaudeE2EGameplayController BeginCapture
#   TargetAutomation.csv   -- from ExecCmds "csvProfile STARTFILE=TargetAutomation.csv"
#   All files reside in Saved/Profiling/CSV/ within the packaged build tree.

param(
    [Parameter(Mandatory)] [string]$WorkspaceDir,    # GitHub workspace root
    [Parameter(Mandatory)] [string]$UE5Root,          # Engine root for PerfreportTool
    [Parameter(Mandatory)] [string]$ReportDir,        # GauntletResults base directory
    [string]$CommitSha = "unknown",                   # For report header
    [string]$RunNumber = "0",                         # For report header
    [switch]$FailOnThreshold                          # Promote threshold violations to errors
)

$ErrorActionPreference = "Stop"

Write-Host "========================================"
Write-Host "Performance Report Generation Pipeline"
Write-Host "========================================"
Write-Host "Workspace:       $WorkspaceDir"
Write-Host "UE5 Root:        $UE5Root"
Write-Host "Report Dir:      $ReportDir"
Write-Host "Commit SHA:      $CommitSha"
Write-Host "Run Number:      $RunNumber"
Write-Host "Fail On Thresh:  $FailOnThreshold"
Write-Host "========================================"

# =============================================================================
# Step 1: Collect CSV files from packaged build's Saved/Profiling/CSV/ directory
# =============================================================================
Write-Host ""
Write-Host "--- Step 1: Collecting CSV profiling files ---"

$perfDataDir = "$ReportDir\PerfData"
New-Item -ItemType Directory -Force -Path $perfDataDir | Out-Null

# Map expected filenames to well-known output names.
# ExactFile: the filename as written by BeginCapture/ExecCmds (no timestamp suffix).
# Plan 16-01 verified UE5 uses the filename as-is when non-empty (CsvProfiler.cpp ~3660).
$nodeMap = @(
    @{Name="BootTest";           ExactFile="BootTest.csv";          Dest="BootTest.csv"},
    @{Name="TargetAutomation";   ExactFile="TargetAutomation.csv";  Dest="TargetAutomation.csv"},
    @{Name="GameplayTest";       ExactFile="GameplayTest.csv";      Dest="GameplayTest.csv"}
)

$collectedNodes = [System.Collections.Generic.List[hashtable]]::new()

foreach ($node in $nodeMap) {
    Write-Host "  Searching for: $($node.ExactFile)"

    # Primary: search for exact filename (Plan 16-01 uses explicit names in BeginCapture)
    $found = Get-ChildItem $WorkspaceDir -Recurse -Filter $node.ExactFile -ErrorAction SilentlyContinue |
             Where-Object { $_.DirectoryName -like "*Profiling*CSV*" } |
             Sort-Object LastWriteTime -Descending |
             Select-Object -First 1

    # Fallback: prefix-wildcard search (handles edge cases where engine appends timestamp)
    if (-not $found) {
        $found = Get-ChildItem $WorkspaceDir -Recurse -Filter "$($node.Name)-*.csv" -ErrorAction SilentlyContinue |
                 Where-Object { $_.DirectoryName -like "*Profiling*CSV*" } |
                 Sort-Object LastWriteTime -Descending |
                 Select-Object -First 1
    }

    # Last resort: any CSV in Profiling/CSV that contains the node name
    if (-not $found) {
        $found = Get-ChildItem $WorkspaceDir -Recurse -Filter "*.csv" -ErrorAction SilentlyContinue |
                 Where-Object { $_.DirectoryName -like "*Profiling*CSV*" -and $_.Name -like "*$($node.Name)*" } |
                 Sort-Object LastWriteTime -Descending |
                 Select-Object -First 1
    }

    if ($found) {
        Copy-Item $found.FullName -Destination "$perfDataDir\$($node.Dest)"
        $sizeKB = [math]::Round($found.Length / 1KB)
        Write-Host "  Collected: $($found.FullName) -> $($node.Dest) ($($sizeKB)KB)"
        $collectedNodes.Add(@{Name=$node.Name; Dest=$node.Dest})
    } else {
        Write-Host "  ::warning::No CSV found for $($node.Name) in Profiling/CSV directories"
    }
}

Write-Host "  Collected $($collectedNodes.Count)/$($nodeMap.Count) CSV files"

# =============================================================================
# Step 2: Run PerfreportTool per collected CSV
# =============================================================================
Write-Host ""
Write-Host "--- Step 2: Running PerfreportTool per CSV ---"

$perfTool = "$UE5Root\Engine\Binaries\DotNET\CsvTools\PerfreportTool.exe"
if (-not (Test-Path $perfTool)) {
    Write-Host "::error::PerfreportTool not found at: $perfTool"
    exit 1
}
Write-Host "  PerfreportTool: $perfTool"

$perfReportsDir = "$ReportDir\PerfReports"
New-Item -ItemType Directory -Force -Path $perfReportsDir | Out-Null

$generatedReports = [System.Collections.Generic.List[string]]::new()

foreach ($node in $collectedNodes) {
    $csvPath      = "$perfDataDir\$($node.Dest)"
    $nodeReportDir = "$perfReportsDir\$($node.Name)"
    New-Item -ItemType Directory -Force -Path $nodeReportDir | Out-Null

    Write-Host "  Processing: $($node.Name)"

    # -reporttype Default30fps: 30fps thresholds appropriate for learning project
    # -writeSummaryCsv: produces summary stats CSV for threshold checking
    # -noWatermarks: omit commandline watermark (contains local paths)
    & "$perfTool" `
        -csv $csvPath `
        -o $nodeReportDir `
        -reporttype Default30fps `
        -title $node.Name `
        -writeSummaryCsv `
        -noWatermarks

    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ::warning::PerfreportTool failed for $($node.Name) (exit code: $LASTEXITCODE)"
    } else {
        Write-Host "  Generated report for $($node.Name) in $nodeReportDir"
        $generatedReports.Add($node.Name)
    }
}

# =============================================================================
# Step 3: Parse summary stats and check thresholds
# =============================================================================
Write-Host ""
Write-Host "--- Step 3: Parsing summary stats and checking thresholds ---"

# Threshold defaults per CONTEXT.md locked decisions:
#   AvgFPS > 30 (for 30fps target), AvgFrameTime < 33.3ms, PeakMemory < 2048MB
$thresholds = @{
    AvgFPS         = @{ Min = 30.0;   Max = $null;  Unit = "fps"; Description = "Average FPS" }
    AvgFrameTimeMs = @{ Min = $null;  Max = 33.3;   Unit = "ms";  Description = "Avg Frame Time" }
    PeakMemoryMB   = @{ Min = $null;  Max = 2048.0; Unit = "MB";  Description = "Peak Memory" }
}

$summaryData  = @{}  # nodeName -> @{AvgFPS, AvgFrameTimeMs, PeakMemoryMB, HitchCount, TotalFrames}
$violations   = [System.Collections.Generic.List[string]]::new()

foreach ($node in $collectedNodes) {
    $csvPath      = "$perfDataDir\$($node.Dest)"
    $nodeReportDir = "$perfReportsDir\$($node.Name)"
    $nodeStats    = @{
        AvgFPS         = $null
        AvgFrameTimeMs = $null
        PeakMemoryMB   = $null
        HitchCount     = 0
        TotalFrames    = 0
    }

    # Try to parse summary CSV written by PerfreportTool -writeSummaryCsv
    # Exact filename is uncertain - use a glob (LOW confidence from research)
    $summaryFile = Get-ChildItem $nodeReportDir -Filter "*summary*" -Recurse -ErrorAction SilentlyContinue |
                   Select-Object -First 1

    if ($summaryFile) {
        Write-Host "  $($node.Name): Using PerfreportTool summary CSV: $($summaryFile.Name)"
        try {
            $summaryRows = Import-Csv $summaryFile.FullName -ErrorAction Stop
            # Summary CSV typically has columns like: stat, avg, min, max, ...
            # Column names may vary; try common patterns
            foreach ($row in $summaryRows) {
                $statName = $row.PSObject.Properties | Where-Object { $_.Name -in @("stat","Stat","name","Name","Metric") } | Select-Object -First 1
                if (-not $statName) { continue }
                $statValue = $statName.Value

                # Look for frametime stat -> compute FPS
                if ($statValue -match "frametime|FrameTime") {
                    $avgCol = $row.PSObject.Properties | Where-Object { $_.Name -in @("avg","Avg","average","Average","Mean","mean") } | Select-Object -First 1
                    if ($avgCol -and [double]::TryParse($avgCol.Value, [ref]$null)) {
                        $ft = [double]$avgCol.Value
                        $nodeStats.AvgFrameTimeMs = $ft
                        if ($ft -gt 0) { $nodeStats.AvgFPS = [math]::Round(1000.0 / $ft, 1) }
                    }
                }
                # Look for memory stat
                if ($statValue -match "memory|Memory|MemoryMB|memoryMB") {
                    $maxCol = $row.PSObject.Properties | Where-Object { $_.Name -in @("max","Max","maximum","Maximum","Peak","peak") } | Select-Object -First 1
                    if ($maxCol -and [double]::TryParse($maxCol.Value, [ref]$null)) {
                        $nodeStats.PeakMemoryMB = [math]::Round([double]$maxCol.Value, 1)
                    }
                }
            }
        } catch {
            Write-Host "  ::warning::Could not parse summary CSV for $($node.Name): $_"
        }
    }

    # Fallback: parse raw per-frame CSV directly for averages
    if ($null -eq $nodeStats.AvgFPS -or $null -eq $nodeStats.AvgFrameTimeMs) {
        Write-Host "  $($node.Name): Falling back to raw CSV parsing"
        try {
            $rawRows = Import-Csv $csvPath -ErrorAction Stop
            $nodeStats.TotalFrames = $rawRows.Count

            # Detect frametime column (UE5 CSV uses "frametime" or "FrameTime")
            $ftCol = $null
            if ($rawRows.Count -gt 0) {
                $ftCol = $rawRows[0].PSObject.Properties |
                         Where-Object { $_.Name -in @("frametime","FrameTime","FrameTimeMs","frameTimeMs") } |
                         Select-Object -First 1 -ExpandProperty Name
            }

            if ($ftCol) {
                $frameTimes = $rawRows | ForEach-Object {
                    $v = 0.0
                    if ([double]::TryParse($_.$ftCol, [ref]$v)) { $v } else { $null }
                } | Where-Object { $null -ne $_ -and $_ -gt 0 }

                if ($frameTimes.Count -gt 0) {
                    $avgFt = ($frameTimes | Measure-Object -Average).Average
                    $nodeStats.AvgFrameTimeMs = [math]::Round($avgFt, 2)
                    if ($avgFt -gt 0) { $nodeStats.AvgFPS = [math]::Round(1000.0 / $avgFt, 1) }

                    # HitchCount: frames where frametime > 50ms (threshold for noticeable hitch)
                    $nodeStats.HitchCount = ($frameTimes | Where-Object { $_ -gt 50.0 }).Count
                }
            }

            # Detect memory column
            $memCol = $null
            if ($rawRows.Count -gt 0) {
                $memCol = $rawRows[0].PSObject.Properties |
                          Where-Object { $_.Name -match "memory|Memory" } |
                          Sort-Object { $_.Name.Length } |
                          Select-Object -First 1 -ExpandProperty Name
            }

            if ($memCol) {
                $memValues = $rawRows | ForEach-Object {
                    $v = 0.0
                    if ([double]::TryParse($_.$memCol, [ref]$v)) { $v } else { $null }
                } | Where-Object { $null -ne $_ -and $_ -gt 0 }

                if ($memValues.Count -gt 0) {
                    $nodeStats.PeakMemoryMB = [math]::Round(($memValues | Measure-Object -Maximum).Maximum, 1)
                }
            }
        } catch {
            Write-Host "  ::warning::Could not parse raw CSV for $($node.Name): $_"
        }
    }

    $summaryData[$node.Name] = $nodeStats

    # Emit parsed stats
    Write-Host "  $($node.Name) stats:"
    Write-Host "    AvgFPS:         $($nodeStats.AvgFPS)"
    Write-Host "    AvgFrameTimeMs: $($nodeStats.AvgFrameTimeMs)"
    Write-Host "    PeakMemoryMB:   $($nodeStats.PeakMemoryMB)"
    Write-Host "    HitchCount:     $($nodeStats.HitchCount)"
    Write-Host "    TotalFrames:    $($nodeStats.TotalFrames)"

    # Check thresholds
    foreach ($key in $thresholds.Keys) {
        $thresh = $thresholds[$key]
        $value  = $nodeStats[$key]

        if ($null -eq $value) { continue }

        $violated = $false
        $msg      = ""

        if ($null -ne $thresh.Min -and $value -lt $thresh.Min) {
            $violated = $true
            $msg = "[$($node.Name)] $($thresh.Description): $value $($thresh.Unit) is below minimum $($thresh.Min) $($thresh.Unit)"
        }
        if ($null -ne $thresh.Max -and $value -gt $thresh.Max) {
            $violated = $true
            $msg = "[$($node.Name)] $($thresh.Description): $value $($thresh.Unit) exceeds maximum $($thresh.Max) $($thresh.Unit)"
        }

        if ($violated) {
            $violations.Add($msg)
            if ($FailOnThreshold) {
                Write-Host "::error::$msg"
            } else {
                Write-Host "::warning::$msg"
            }
        }
    }
}

Write-Host "  Threshold violations: $($violations.Count)"

# =============================================================================
# Step 4: Generate master performance-report.html
# =============================================================================
Write-Host ""
Write-Host "--- Step 4: Generating master performance-report.html ---"

$reportDate       = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss UTC")
$thresholdMode    = if ($FailOnThreshold) { "ENFORCED" } else { "ADVISORY" }
$violationCount   = $violations.Count

# Build dashboard rows (one per collected node)
$dashboardRows = ""
foreach ($node in $collectedNodes) {
    $stats     = $summaryData[$node.Name]
    $nodeName  = $node.Name

    # Helper: format value or N/A
    $fmtFPS    = if ($null -ne $stats.AvgFPS)         { "$($stats.AvgFPS) fps"  } else { "N/A" }
    $fmtFT     = if ($null -ne $stats.AvgFrameTimeMs) { "$($stats.AvgFrameTimeMs) ms" } else { "N/A" }
    $fmtMem    = if ($null -ne $stats.PeakMemoryMB)   { "$($stats.PeakMemoryMB) MB"  } else { "N/A" }
    $fmtHitch  = "$($stats.HitchCount)"
    $fmtFrames = "$($stats.TotalFrames)"

    # Determine cell colors for threshold-checked metrics
    function Get-CellClass {
        param([string]$metric, $value)
        if ($null -eq $value) { return "na" }
        switch ($metric) {
            "AvgFPS" {
                if ($value -lt 30) { return "fail" }
                if ($value -lt 40) { return "warn" }
                return "pass"
            }
            "AvgFrameTimeMs" {
                if ($value -gt 33.3) { return "fail" }
                if ($value -gt 25.0) { return "warn" }
                return "pass"
            }
            "PeakMemoryMB" {
                if ($value -gt 2048) { return "fail" }
                if ($value -gt 1500) { return "warn" }
                return "pass"
            }
            default { return "na" }
        }
    }

    $fpsClass = Get-CellClass "AvgFPS"         $stats.AvgFPS
    $ftClass  = Get-CellClass "AvgFrameTimeMs" $stats.AvgFrameTimeMs
    $memClass = Get-CellClass "PeakMemoryMB"   $stats.PeakMemoryMB

    $dashboardRows += "        <tr>`n"
    $dashboardRows += "          <td><strong>$nodeName</strong></td>`n"
    $dashboardRows += "          <td class=`"$fpsClass`">$fmtFPS</td>`n"
    $dashboardRows += "          <td class=`"$ftClass`">$fmtFT</td>`n"
    $dashboardRows += "          <td class=`"$memClass`">$fmtMem</td>`n"
    $dashboardRows += "          <td>$fmtHitch</td>`n"
    $dashboardRows += "          <td>$fmtFrames</td>`n"
    $dashboardRows += "        </tr>`n"
}

if ($dashboardRows -eq "") {
    $dashboardRows = "        <tr><td colspan=`"6`" style=`"text-align:center`">No CSV data collected.</td></tr>`n"
}

# Build threshold check rows
$thresholdRows = ""
foreach ($key in @("AvgFPS","AvgFrameTimeMs","PeakMemoryMB")) {
    $thresh = $thresholds[$key]
    foreach ($node in $collectedNodes) {
        $stats = $summaryData[$node.Name]
        $value = $stats[$key]
        if ($null -eq $value) {
            $status = "<span class=`"badge-na`">N/A</span>"
            $detail = "No data"
        } else {
            $violated = $false
            if ($null -ne $thresh.Min -and $value -lt $thresh.Min) { $violated = $true }
            if ($null -ne $thresh.Max -and $value -gt $thresh.Max) { $violated = $true }
            if ($violated) {
                $status = "<span class=`"badge-fail`">FAIL</span>"
            } else {
                $status = "<span class=`"badge-pass`">PASS</span>"
            }
            $limitStr = if ($null -ne $thresh.Min) { "min: $($thresh.Min)" } else { "max: $($thresh.Max)" }
            $detail = "$value $($thresh.Unit) ($limitStr $($thresh.Unit))"
        }
        $thresholdRows += "        <tr>`n"
        $thresholdRows += "          <td>$($node.Name)</td>`n"
        $thresholdRows += "          <td>$($thresh.Description)</td>`n"
        $thresholdRows += "          <td>$detail</td>`n"
        $thresholdRows += "          <td>$status</td>`n"
        $thresholdRows += "        </tr>`n"
    }
}

if ($thresholdRows -eq "") {
    $thresholdRows = "        <tr><td colspan=`"4`" style=`"text-align:center`">No threshold data available.</td></tr>`n"
}

# Build per-node iframe sections
$iframeSections = ""
foreach ($node in $nodeMap) {
    $nodeName = $node.Name
    $hasData  = ($collectedNodes | Where-Object { $_.Name -eq $nodeName }).Count -gt 0

    $iframeSections += "    <section class=`"node-section`">`n"
    $iframeSections += "      <h2>$nodeName</h2>`n"

    if ($hasData) {
        $iframeSections += "      <iframe src=`"$nodeName/index.html`" width=`"100%`" height=`"800`" frameborder=`"0`" loading=`"lazy`">`n"
        $iframeSections += "        <p><a href=`"$nodeName/index.html`">View $nodeName report</a></p>`n"
        $iframeSections += "      </iframe>`n"
    } else {
        $iframeSections += "      <div class=`"no-data`">No data collected for this node. CSV profiling may not have produced output.</div>`n"
    }

    $iframeSections += "    </section>`n"
}

# Build crash events section (conditional)
$crashSection = ""
$crashArtifactsDir = "$ReportDir\CrashArtifacts"
if (Test-Path $crashArtifactsDir) {
    $crashFiles = Get-ChildItem $crashArtifactsDir -Recurse -ErrorAction SilentlyContinue
    if ($crashFiles -and $crashFiles.Count -gt 0) {
        $crashRows = ""
        foreach ($cf in $crashFiles) {
            $sizeStr = if ($cf.Length -gt 1MB) {
                "$([math]::Round($cf.Length / 1MB, 1)) MB"
            } elseif ($cf.Length -gt 1KB) {
                "$([math]::Round($cf.Length / 1KB, 0)) KB"
            } else {
                "$($cf.Length) bytes"
            }
            $ts = $cf.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss")
            $crashRows += "        <tr><td>$($cf.Name)</td><td>$sizeStr</td><td>$ts</td></tr>`n"
        }

        $crashSection = @"
    <section class="crash-section">
      <h2>Crash Events</h2>
      <p>Crash artifacts were collected during this run. These files are also available as the <code>crash-artifacts</code> CI artifact.</p>
      <table>
        <thead><tr><th>File</th><th>Size</th><th>Timestamp</th></tr></thead>
        <tbody>
$crashRows        </tbody>
      </table>
    </section>
"@
    }
}

# Violations summary for header
$violationBadge = if ($violationCount -gt 0) {
    "<span class=`"badge-fail`">$violationCount violation$(if ($violationCount -ne 1) { 's' })</span>"
} else {
    "<span class=`"badge-pass`">All thresholds passed</span>"
}

# Write master HTML report
$htmlContent = @"
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Performance Report - Commit $CommitSha - Run #$RunNumber</title>
  <style>
    /* Reset and base */
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      font-size: 14px;
      line-height: 1.5;
      color: #24292e;
      background: #f6f8fa;
      padding: 24px;
    }
    a { color: #0366d6; text-decoration: none; }
    a:hover { text-decoration: underline; }

    /* Layout */
    .container { max-width: 1200px; margin: 0 auto; }
    section { background: #fff; border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; margin-bottom: 20px; }
    h1 { font-size: 24px; font-weight: 600; margin-bottom: 8px; }
    h2 { font-size: 18px; font-weight: 600; margin-bottom: 12px; border-bottom: 1px solid #e1e4e8; padding-bottom: 8px; }
    h3 { font-size: 14px; font-weight: 600; margin-bottom: 8px; }

    /* Header */
    .report-header { background: #fff; border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; margin-bottom: 20px; }
    .report-header .meta { color: #586069; font-size: 12px; margin-top: 4px; }
    .report-header .badges { margin-top: 10px; }

    /* Tables */
    table { width: 100%; border-collapse: collapse; margin-top: 8px; font-size: 13px; }
    th { background: #f6f8fa; font-weight: 600; text-align: left; padding: 8px 10px; border: 1px solid #e1e4e8; }
    td { padding: 8px 10px; border: 1px solid #e1e4e8; }
    tr:nth-child(even) td { background: #fafbfc; }

    /* Threshold color coding */
    td.pass { background: #dcffe4 !important; color: #155724; font-weight: 600; }
    td.warn { background: #fff3cd !important; color: #856404; font-weight: 600; }
    td.fail { background: #ffdce0 !important; color: #721c24; font-weight: 600; }
    td.na   { color: #6a737d; }

    /* Badges */
    .badge-pass { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #28a745; color: #fff; }
    .badge-fail { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #d73a49; color: #fff; }
    .badge-warn { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #e36209; color: #fff; }
    .badge-na   { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #6a737d; color: #fff; }
    .badge-mode { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #0366d6; color: #fff; }

    /* Node sections */
    .node-section iframe { display: block; width: 100%; height: 800px; border: 1px solid #e1e4e8; border-radius: 4px; margin-top: 8px; }
    .no-data { padding: 20px; background: #f6f8fa; border: 1px dashed #d1d5da; border-radius: 4px; color: #586069; text-align: center; }

    /* Crash section */
    .crash-section { border-color: #d73a49; }
    .crash-section h2 { color: #d73a49; }

    /* Footer */
    .footer { text-align: center; color: #6a737d; font-size: 12px; padding: 16px 0; }
  </style>
</head>
<body>
  <div class="container">

    <!-- Header -->
    <div class="report-header">
      <h1>Gauntlet E2E Performance Report</h1>
      <div class="meta">Commit: <strong>$CommitSha</strong> &nbsp;|&nbsp; Run: <strong>#$RunNumber</strong> &nbsp;|&nbsp; Generated: $reportDate</div>
      <div class="badges">
        Threshold mode: <span class="badge-mode">$thresholdMode</span>
        &nbsp; Violations: $violationBadge
      </div>
    </div>

    <!-- Dashboard Summary -->
    <section>
      <h2>Dashboard Summary</h2>
      <table>
        <thead>
          <tr>
            <th>Test Node</th>
            <th>Avg FPS</th>
            <th>Avg Frame Time</th>
            <th>Peak Memory</th>
            <th>Hitch Count (&gt;50ms)</th>
            <th>Total Frames</th>
          </tr>
        </thead>
        <tbody>
$dashboardRows        </tbody>
      </table>
      <p style="margin-top:8px; font-size:12px; color:#586069;">
        Green = pass &nbsp; Yellow = within 80% of threshold &nbsp; Red = threshold violated
      </p>
    </section>

    <!-- Threshold Checks -->
    <section>
      <h2>Threshold Checks</h2>
      <p style="margin-bottom:8px;">
        Mode: <span class="badge-mode">$thresholdMode</span>
        $(if ($thresholdMode -eq "ENFORCED") { " &mdash; Violations cause CI failure (PERF_FAIL_ON_THRESHOLD=true)" } else { " &mdash; Violations emit warnings only (set PERF_FAIL_ON_THRESHOLD=true to enforce)" })
      </p>
      <table>
        <thead>
          <tr><th>Node</th><th>Metric</th><th>Value (Limit)</th><th>Status</th></tr>
        </thead>
        <tbody>
$thresholdRows        </tbody>
      </table>
    </section>

$iframeSections
$crashSection
    <!-- Footer -->
    <div class="footer">
      Generated by Generate-PerfReport.ps1 &nbsp;|&nbsp; $reportDate
    </div>

  </div>
</body>
</html>
"@

$masterHtmlPath = "$perfReportsDir\performance-report.html"
$htmlContent | Out-File -FilePath $masterHtmlPath -Encoding utf8
Write-Host "  Master HTML written to: $masterHtmlPath"

# =============================================================================
# Step 5: Output summary and return exit code
# =============================================================================
Write-Host ""
Write-Host "--- Step 5: Summary ---"
Write-Host "  CSV files collected:    $($collectedNodes.Count)"
Write-Host "  HTML reports generated: $($generatedReports.Count)"
Write-Host "  Threshold violations:   $violationCount"
Write-Host "  Master HTML:            $masterHtmlPath"

# Output violation count to GITHUB_OUTPUT if available
if ($env:GITHUB_OUTPUT) {
    "PERF_VIOLATIONS_COUNT=$violationCount" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    Write-Host "  PERF_VIOLATIONS_COUNT=$violationCount written to GITHUB_OUTPUT"
}

if ($FailOnThreshold -and $violationCount -gt 0) {
    Write-Host ""
    Write-Host "::error::Performance threshold violations detected ($violationCount). Failing due to -FailOnThreshold."
    exit 1
}

Write-Host ""
Write-Host "Performance report generation complete."
exit 0

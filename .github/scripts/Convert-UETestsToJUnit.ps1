# Convert-UETestsToJUnit.ps1
# Converts UE5 Automation System JSON report to JUnit XML format
# Usage: .\Convert-UETestsToJUnit.ps1 -JsonPath "path\to\index.json" -OutputPath "path\to\results.xml"

param(
    [Parameter(Mandatory=$true)]
    [string]$JsonPath,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath
)

$ErrorActionPreference = "Stop"

# Read JSON and strip UTF-8 BOM (UE5 outputs with BOM)
$jsonContent = Get-Content -Path $JsonPath -Raw -Encoding UTF8
$jsonContent = $jsonContent.TrimStart([char]0xFEFF)
$jsonContent = $jsonContent -replace '^\xEF\xBB\xBF', ''

$report = $jsonContent | ConvertFrom-Json

# Build JUnit XML
$xml = [System.Xml.XmlDocument]::new()
$declaration = $xml.CreateXmlDeclaration("1.0", "UTF-8", $null)
$xml.AppendChild($declaration) | Out-Null

$testsuite = $xml.CreateElement("testsuite")
$testsuite.SetAttribute("name", "UE5 Automation Tests")
$totalTests = $report.succeeded + $report.succeededWithWarnings + $report.failed + $report.notRun
$testsuite.SetAttribute("tests", $totalTests)
$testsuite.SetAttribute("failures", $report.failed)
$testsuite.SetAttribute("errors", 0)
$testsuite.SetAttribute("skipped", $report.notRun)
$testsuite.SetAttribute("time", $report.totalDuration)

foreach ($test in $report.tests) {
    $testcase = $xml.CreateElement("testcase")
    $testcase.SetAttribute("name", $test.testDisplayName)
    $testcase.SetAttribute("classname", $test.fullTestPath)

    # Per-test duration if available, otherwise 0
    $duration = 0
    if ($test.PSObject.Properties["duration"]) {
        $duration = $test.duration
    }
    $testcase.SetAttribute("time", $duration)

    if ($test.state -eq "Fail") {
        foreach ($entry in $test.entries) {
            if ($entry.event.type -eq "Error") {
                $failure = $xml.CreateElement("failure")
                $failure.SetAttribute("message", $entry.event.message)
                $failure.SetAttribute("type", "AssertionError")

                # Include file:line if available
                $detail = ""
                if ($entry.filename -and $entry.lineNumber) {
                    $detail = "$($entry.filename):$($entry.lineNumber)"
                }
                $failure.InnerText = $detail
                $testcase.AppendChild($failure) | Out-Null
            }
        }
    }
    elseif ($test.state -eq "NotRun" -or $test.state -eq "Skipped") {
        $skipped = $xml.CreateElement("skipped")
        $testcase.AppendChild($skipped) | Out-Null
    }

    $testsuite.AppendChild($testcase) | Out-Null
}

$xml.AppendChild($testsuite) | Out-Null
$xml.Save($OutputPath)

Write-Host "Converted $totalTests tests to JUnit XML: $OutputPath"
Write-Host "  Passed: $($report.succeeded)"
Write-Host "  Failed: $($report.failed)"
Write-Host "  Skipped: $($report.notRun)"

# Return failure count for exit code handling
return $report.failed

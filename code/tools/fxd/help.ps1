<#
.Synopsis
The help command.
#>

function Get-Sections($text)
{
    $sectionHash = @{} # collection sections keyed by section name
    $sectionOrder = @() # remember insertion order
    $rowNum = 0
    $lowBound = 0
    $sectionName = ""
    # Handle corner case of no help defined for a given function,
    # where help returned just 1 row containing the syntax without an indent.
    if ($helpText.Count -eq 1) { $sectionName = $SYNTAX_SECTION; $rowNum = 1 }
    else {
    $text | ForEach-Object {
        # The normal help text has section headers (NAME, SYNOPSIS, SYNTAX, DESCRIPTION, etc.)
        # at the start of a line and everything else indented.
        # Thus, this signals a new section:
        if ($_ -match "^[A-Z]") {
            Add-HelpSection $sectionName $text $lowBound ([ref]$sectionHash) ([ref]$sectionOrder)# output prior section
            $sectionName = $_
            $lowBound = $rowNum + 1
        }
        # Add separate section title for examples (which standard help lacks).
        elseif ($_ -match "----\s+EXAMPLE 1\s+----") {
            Add-HelpSection $sectionName $text $lowBound ([ref]$sectionHash) ([ref]$sectionOrder)# output prior section
            $sectionName = $EXAMPLES_SECTION
            $lowBound = $rowNum
        }
        $rowNum++
    }
    }
    Add-HelpSection $sectionName $text $lowBound ([ref]$sectionHash) ([ref]$sectionOrder)# output final section
    $sectionHash, $sectionOrder
}

function Add-HelpSection($sectionName, $text, $lowBound, [ref]$hash, [ref]$order)
{
    if ($sectionName) { # output previously collected section
        $hash.value[$sectionName] = $text[$lowBound..($rowNum-1)]
        $order.value += $sectionName
    }
}

"# fxd: the cfx.re developer utility"
"# https://cfx.re/"

""

"usage: fxd <subcommand> [args]"

foreach ($CommandScript in Get-ChildItem $PSScriptRoot) {
	if ($CommandScript.BaseName[0] -eq '.') {
		continue;
	}

	$helps = Get-Sections (Get-Help $CommandScript | Out-String -Stream -Width 16384)
	"- fxd $($CommandScript.BaseName): $($helps.Synopsis?.Trim() ?? 'No help.')" 
}

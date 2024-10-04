function Invoke-LogSection {
    param(
        [Parameter(Position = 0)][string]$DisplayName,
        [Parameter(Position = 1)][scriptblock]$Block
    )

    $Id = ConvertTo-Slug $DisplayName

    $esc = $([char]27)

    $purple = "$esc[95m"
    $reset = "$esc[0m"
    
    Write-Host "section_start:$(Get-Date -UFormat %s -Millisecond 0):$Id`r$esc[0K${purple}$DisplayName${reset}"

    & $Block

    Write-Host "section_end:$(Get-Date -UFormat %s -Millisecond 0):$Id`r$esc[0K"
    Write-Host "${purple}[DONE] $DisplayName${reset}"
}

#########################################################################################
# KNS Helper Module
# 
# Copyright 2019, Nicolas Kapfer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
# and associated documentation files (the "Software"), to deal in the Software without restriction, 
# including without limitation the rights to use, copy, modify, merge, publish, distribute, 
# sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or 
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
# BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#########################################################################################

Function ConvertTo-Slug 
{
    <#
    .SYNOPSIS
    Slugify a String in entry
    
    .DESCRIPTION
    This function "slugifies" a string given in entry 
    
    .PARAMETER Text
    String. Text to slugify
    
    .PARAMETER Delimiter
    String. Optional, the delimiter used. If omitted, "-" will be used

    .PARAMETER CapitalizeFirstLetter
    Switch. If the switch is set, it will capitalize the first letter of each word in the "Text" string.
    
    .EXAMPLE
    PS> ConvertTo-Slug "That's all folks!"
    Will return "thats-all-folks"

    .EXAMPLE
    PS> ConvertTo-Slug "That's all Folks!" -Delimiter "_"
    Will return "thats_all_folks"

    .EXAMPLE
    PS> ConvertTo-Slug "That's all Folks!" -Delimiter "" -CapitalizeFirstLetter
    Will return "ThatsAllFolks"
    
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$True,ValueFromPipeline=$True,Position=0)]
        [String]
        $Text,

        [Parameter(Mandatory=$False,Position=1)]
        [string]
        $Delimiter="-",

        [Parameter()]
        [Switch]
        $CapitalizeFirstLetter
    )

    $bytes = [System.Text.Encoding]::GetEncoding("Cyrillic").GetBytes($text)
    $result = [System.Text.Encoding]::ASCII.GetString($bytes).ToLower()


    if($CapitalizeFirstLetter){
        $TextInfo = (Get-Culture).TextInfo
        $result = $TextInfo.ToTitleCase($result)
    }

    $rx = [System.Text.RegularExpressions.Regex]
    $result = $rx::Replace($result, "[^a-zA-Z0-9\s-]", "")
    $result = $rx::Replace($result, "\s+", " ").Trim(); 
    $result = $rx::Replace($result, "\s", $Delimiter);
    Write-Output $result
}
# hp8470p

if (!($Hp8470pSession) -or ($Hp8470pSession.State -eq "Broken")) {
    $Hp8470pSession = (. $Scripts\NewHp8470pSession.ps1)
}

$TargetSessionPath = "C:\DriverTest\Drivers\ProcessMemoryScanner\"
$TargetSession = $Hp8470pSession
$ItemsToDeploy = @("*.exe", "*.ps1")

try
{
    Invoke-Command -Session $TargetSession {
        New-Item -ItemType "Directory" -Force $Using:TargetSessionPath
    }

    $ItemsToDeploy | Foreach-Object {
        Write-Host "Deploying item: " $_
        Copy-Item -Recurse -Force -Path $_ -ToSession $TargetSession -Destination $TargetSessionPath
    }

    Invoke-Command -Session $TargetSession {
        cd $Using:TargetSessionPath
    }
}
catch
{
    Write-Error $_
}

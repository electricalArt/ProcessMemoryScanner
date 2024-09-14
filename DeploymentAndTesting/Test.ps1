$Global:LastExitCode = 0
$Value = 1666
$ValueType = "int32"

$TargetProcessId = $null

function StartTargetProcess() {
    Write-Host
    Write-Host "StartProcess()"
    Write-Host
    $script:TargetProcessId = `
        (Start-Process .\FixedMemoryApplication.exe `
            -ArgumentList @($ValueType, $Value) -PassThru
        ).Id
    if ($LastExitCode) {
        throw $LastExitCode
    }
    Start-Sleep 0.05
}
function TestSearch() {
    .\ProcessMemoryScanner.exe search $Value --process-id $TargetProcessId --value-type $ValueType
}
function TestFilter() {
    .\ProcessMemoryScanner.exe filter ($Value - 1) --process-id $TargetProcessId --value-type $ValueType
}
function TestWrite() {
    .\ProcessMemoryScanner.exe write 1000 --process-id $TargetProcessId --value-type $ValueType
}
function StopTargetProcess() {
    Write-Host
    Write-Host "StopTargetProcess()"
    Write-Host
    Stop-Process -Id $TargetProcessId
}

StartTargetProcess
TestSearch
#TestFilter
#TestWrite
StopTargetProcess

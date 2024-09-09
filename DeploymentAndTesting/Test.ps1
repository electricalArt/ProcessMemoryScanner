$Value = 1499
$ProcessId = 14992
$ValueType = "int32"
#$OutputFile = ".\addresses.txt"

function TestSearch() {
    param(
        $Value
    )
    .\ProcessMemoryScanner.exe search $Value --process-id $ProcessId --value-type $ValueType
}
function TestFilter() {
    .\ProcessMemoryScanner.exe filter ($Value - 1) --process-id $ProcessId --value-type $ValueType
}
function TestWrite() {
    .\ProcessMemoryScanner.exe write 1000 --process-id $ProcessId --value-type $ValueType
}

TestSearch -Value $Value
#TestFilter
#TestWrite

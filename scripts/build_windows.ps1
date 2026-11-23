# Run this from the project root inside the VS Code PowerShell terminal:
# .\scripts\build_windows.ps1
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
$allSources = @('src/main.c') + @(
    Get-ChildItem 'src/core/*.c','src/algorithms/*.c' | ForEach-Object { $_.FullName }
)
& gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude $allSources -o main.exe
if ($LASTEXITCODE -ne 0) { throw 'Main project compilation failed' }
$testSources = @('tests/test_main.c') + @(
    Get-ChildItem 'src/core/*.c','src/algorithms/*.c' | ForEach-Object { $_.FullName }
)
& gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude $testSources -o port_tests.exe
if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
Write-Host 'Build OK. Run: .\main.exe --menu'
Write-Host 'Tests:    .\port_tests.exe'

$ErrorActionPreference = "Stop"

$w64devkit = "C:\1projects\w64devkit\bin"
if (Test-Path $w64devkit) {
    $env:PATH = "$w64devkit;$env:PATH"
} else {
    Write-Host "ERROR: w64devkit not found at $w64devkit" -ForegroundColor Red
    Write-Host "Download from https://github.com/skeeto/w64devkit/releases and extract to C:\1projects\w64devkit"
    exit 1
}

Write-Host "Compiling Air Travel DB..." -ForegroundColor Cyan
g++ -std=c++17 -O2 -I include src/DataManager.cpp src/main.cpp -o airtraveldb.exe -lws2_32 -DNOMINMAX -D_WIN32_WINNT=0x0A00

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful! Run with: .\airtraveldb.exe" -ForegroundColor Green
    Write-Host "Then open http://localhost:8080 in your browser." -ForegroundColor Yellow
} else {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
}

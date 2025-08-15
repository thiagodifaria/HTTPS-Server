param (
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release"
)

$PSDefaultParameterValues['*:Encoding'] = 'utf8'

Write-Host "Limpeza: Removendo o diretório de build antigo..." -ForegroundColor Yellow
if (Test-Path -Path "build") {
    Remove-Item -Recurse -Force "build"
}

Write-Host "Configuração: Executando o CMake para o tipo '$Configuration'..." -ForegroundColor Cyan
cmake -B build -DCMAKE_BUILD_TYPE=$Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO: A configuração do CMake falhou." -ForegroundColor Red
    exit 1
}

Write-Host "Compilação: Construindo o projeto com a configuração '$Configuration'..." -ForegroundColor Green
cmake --build build --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO: A compilação falhou." -ForegroundColor Red
    exit 1
}

Write-Host "SUCESSO: O projeto foi compilado com êxito em 'build'." -ForegroundColor Green
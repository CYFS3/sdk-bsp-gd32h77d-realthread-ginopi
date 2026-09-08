param([string]$Compiler = 'C:\msys64\ucrt64\bin\gcc.exe')
$ErrorActionPreference = 'Stop'
$sourcePath = Join-Path $PSScriptRoot '..\applications\board_demo_backend.c'
$source = [IO.File]::ReadAllText($sourcePath)

function Get-SourceSection([string]$Begin, [string]$End) {
    $start = $source.IndexOf($Begin, [StringComparison]::Ordinal)
    if ($start -lt 0) { throw "Missing source marker: $Begin" }
    $stop = $source.IndexOf($End, $start + $Begin.Length, [StringComparison]::Ordinal)
    if ($stop -lt 0) { throw "Missing source marker: $End" }
    return $source.Substring($start, $stop - $start)
}

$generated = Get-SourceSection 'enum board_demo_command_type' 'static struct board_demo_snapshot demo_state;'
$generated += ([regex]::Matches($source, '(?m)^#define BOARD_DEMO_I2C_[^\r\n]+') |
    ForEach-Object { $_.Value }) -join "`n"
$generated += "`n"
$generated += Get-SourceSection 'static void board_demo_strncpy(' 'int board_demo_fs_source('
$generated += Get-SourceSection 'static rt_ssize_t board_demo_i2c_probe(' '#endif /* RT_USING_I2C */'
$generated += Get-SourceSection 'static void board_demo_worker_entry(' 'int board_demo_backend_init('

$testDir = Join-Path ([IO.Path]::GetTempPath()) ('gino-i2c-tests-' + [guid]::NewGuid())
[IO.Directory]::CreateDirectory($testDir) | Out-Null
[IO.File]::WriteAllText((Join-Path $testDir 'board_demo_test_functions.h'), $generated)
$testExe = Join-Path $testDir 'board_demo_tests.exe'
$compilerArgs = @('-std=c99', '-Wall', '-Wextra', '-Werror',
    ('-I' + (Join-Path $PSScriptRoot 'stubs')),
    ('-I' + (Join-Path $PSScriptRoot '..\applications')),
    ('-I' + $testDir), (Join-Path $PSScriptRoot 'test_board_demo_backend.c'), '-o', $testExe)
$originalPath = $env:PATH
try {
    $env:PATH = (Split-Path (Get-Command $Compiler).Source) + ';' + $originalPath
    & $Compiler @compilerArgs
    if ($LASTEXITCODE -ne 0) { throw "Host compilation failed: $LASTEXITCODE" }
    & $testExe
    if ($LASTEXITCODE -ne 0) { throw "Host tests failed: $LASTEXITCODE" }
}
finally {
    $env:PATH = $originalPath
}

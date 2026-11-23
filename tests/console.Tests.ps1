param(
    [Parameter(Mandatory)]
    [string] $Executable
)

$ErrorActionPreference = 'Stop'
$Executable = (Resolve-Path -LiteralPath $Executable).Path
$testDirectory = New-Item -ItemType Directory -Path (
    Join-Path ([System.IO.Path]::GetTempPath()) "student-management-$([guid]::NewGuid())"
)
$data = Join-Path $testDirectory.FullName 'students.txt'

function Invoke-StudentManagement([string[]] $Lines, [int] $ExpectedExit = 0) {
    $start = [System.Diagnostics.ProcessStartInfo]::new($Executable)
    $start.WorkingDirectory = $testDirectory.FullName
    $start.UseShellExecute = $false
    $start.RedirectStandardInput = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.StandardInputEncoding = [System.Text.UTF8Encoding]::new($false)
    $start.StandardOutputEncoding = [System.Text.Encoding]::UTF8
    $start.StandardErrorEncoding = [System.Text.Encoding]::UTF8
    $process = [System.Diagnostics.Process]::Start($start)
    try {
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        foreach ($line in $Lines) {
            $process.StandardInput.WriteLine($line)
        }
        $process.StandardInput.Close()
        if (!$process.WaitForExit(10000)) {
            $process.Kill()
            throw 'The program did not exit within 10 seconds.'
        }
        if ($process.ExitCode -ne $ExpectedExit) {
            throw "Expected exit $ExpectedExit, got $($process.ExitCode). $($stderr.Result)"
        }
        $stdout.Result + $stderr.Result
    } finally {
        $process.Dispose()
    }
}

function Assert-Contains([string] $Output, [string] $Expected) {
    if (!$Output.Contains($Expected)) {
        throw "Expected output to contain '$Expected'. Output: $Output"
    }
}

try {
    $output = Invoke-StudentManagement @(
        'x', '1', 'no-id', '1', '-1', '1', '17', '   ',
        '1', '17', '  Анна Петрова  ', '1', '17',
        '5', '17', '   ', '6', '17', '20x', '6', '17', '21',
        '6', '17', '10.5', '6', '17', '20', '2', '0'
    )
    Assert-Contains $output 'Ошибка ввода!'
    Assert-Contains $output 'Ошибка ввода ID!'
    Assert-Contains $output 'ID должен быть положительным.'
    Assert-Contains $output 'Имя не может быть пустым.'
    Assert-Contains $output 'Студент с таким ID уже существует.'
    Assert-Contains $output 'Оценка должна быть от 0 до 20.'
    Assert-Contains $output 'Всего студентов: 1'
    Assert-Contains $output 'ID: 17 | Имя: Анна Петрова | Оценки: 10.5 20 | Средний балл: 15.25'
    if ((Select-String -Path $data -Pattern '^2$').Count -ne 1) {
        throw 'Expected two stored grades.'
    }

    $output = Invoke-StudentManagement @('3', '17', '7', 'Петров', '4', '17', 'n', '0')
    Assert-Contains $output 'Удаление отменено.'
    if ([regex]::Matches($output, 'ID: 17 \| Имя: Анна Петрова \| Оценки: 10\.5 20').Count -ne 2) {
        throw 'Expected matching results from ID and name searches after restarting.'
    }

    $output = Invoke-StudentManagement @('5', '17', '  Анна Смирнова ', '2', '0')
    Assert-Contains $output 'ID: 17 | Имя: Анна Смирнова | Оценки: 10.5 20'
    $output = Invoke-StudentManagement @('4', '17', 'y', '2', '0')
    Assert-Contains $output 'Студент успешно удалён!'
    Assert-Contains $output 'Всего студентов: 0'
    $output = Invoke-StudentManagement @('2')
    Assert-Contains $output 'Список студентов пуст.'
    Write-Output 'PASS: console input, searches, student editing, grade persistence, deletion and EOF'
} finally {
    Remove-Item -LiteralPath $data -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $testDirectory.FullName
}

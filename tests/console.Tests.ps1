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

function Invoke-StudentManagement([string[]] $Lines, [int] $ExpectedExit = 0, [string[]] $Arguments = @()) {
    $start = [System.Diagnostics.ProcessStartInfo]::new($Executable)
    $start.WorkingDirectory = $testDirectory.FullName
    $start.UseShellExecute = $false
    $start.RedirectStandardInput = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.StandardInputEncoding = [System.Text.UTF8Encoding]::new($false)
    $start.StandardOutputEncoding = [System.Text.Encoding]::UTF8
    $start.StandardErrorEncoding = [System.Text.Encoding]::UTF8
    foreach ($argument in $Arguments) {
        $start.ArgumentList.Add($argument)
    }
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

    $invalidFiles = @(
        "17`nAnna`n2`n12`n",
        "17`nAnna`n1`n99`n",
        "17`nAnna`n0`n17`nMaria`n0`n"
    )
    foreach ($invalid in $invalidFiles) {
        [System.IO.File]::WriteAllText($data, $invalid)
        $output = Invoke-StudentManagement @() -ExpectedExit 1
        Assert-Contains $output 'Не удалось прочитать students.txt.'
        if ([System.IO.File]::ReadAllText($data) -cne $invalid) {
            throw 'Invalid data was unexpectedly modified.'
        }
    }

    $valid = "17`nAnna`n1`n12.123456789012345`n"
    [System.IO.File]::WriteAllText($data, $valid)
    $output = Invoke-StudentManagement @('1', '18', 'Maria', '0')
    $storedGrade = ([System.IO.File]::ReadAllLines($data))[3]
    if ([double]$storedGrade -ne 12.123456789012345) {
        throw 'Saving a different student changed the stored grade.'
    }
    $before = [System.IO.File]::ReadAllText($data)
    $lock = [System.IO.File]::Open($data, 'Open', 'Read', 'Read')
    try {
        $output = Invoke-StudentManagement @('6', '17', '19') -ExpectedExit 1
        Assert-Contains $output 'Ошибка сохранения файла!'
        if ($output.Contains('Оценка успешно добавлена!')) {
            throw 'The program reported success after a failed save.'
        }
    } finally {
        $lock.Dispose()
    }
    if ([System.IO.File]::ReadAllText($data) -cne $before) {
        throw 'A failed replacement modified the original data.'
    }
    if (Get-ChildItem -LiteralPath $testDirectory.FullName -Filter 'stu*.tmp') {
        throw 'A failed replacement left a temporary file.'
    }
    Write-Output 'PASS: corrupt data protection, grade precision and failed file replacement'

    $validData = [System.IO.File]::ReadAllText($data)
    [System.IO.File]::WriteAllText($data, $validData, [System.Text.UTF8Encoding]::new($true))
    $output = Invoke-StudentManagement @('3', '17', '0')
    Assert-Contains $output 'ID: 17 | Имя: Anna | Оценки:'
    Write-Output 'PASS: UTF-8 data with a byte order mark'

    $output = Invoke-StudentManagement @(
        '8', '17', '-1', '8', '17', '1', '50', '8', '17', '1', '16.5',
        '9', '17', '1', 'n', '6', '17', '9.5', '9', '17', '2', 'Y',
        '1', '3', 'Борис, "Б"', '1', '4', '=2+2', '2', '10', '11', '0'
    )
    Assert-Contains $output 'Неверный номер оценки.'
    Assert-Contains $output 'Ошибка! Оценка должна быть от 0 до 20.'
    Assert-Contains $output 'Оценка изменена.'
    Assert-Contains $output 'Удаление отменено.'
    Assert-Contains $output 'Оценка удалена.'
    Assert-Contains $output 'Студентов с оценками: 1 / 4'
    Assert-Contains $output 'Всего оценок: 1'
    Assert-Contains $output 'Среднее всех оценок: 16.5'
    Assert-Contains $output 'Минимум: 16.5 | Максимум: 16.5'
    $listedIds = [regex]::Matches($output, 'ID: (\d+) \| Имя:') | ForEach-Object { $_.Groups[1].Value }
    if (($listedIds -join ',') -ne '3,4,17,18') {
        throw 'Students were not listed in ascending ID order.'
    }

    $report = "$data.csv"
    $rows = Import-Csv -LiteralPath $report
    $row = $rows | Where-Object id -eq '3'
    if ($row.name -cne 'Борис, "Б"' -or $row.grade_count -ne '0' -or $row.average -ne '') {
        throw 'CSV did not preserve the escaped name or blank average.'
    }
    $row = $rows | Where-Object id -eq '4'
    if ($row.name -cne "'=2+2") {
        throw 'CSV did not neutralize the formula-like name.'
    }
    $row = $rows | Where-Object id -eq '17'
    if ($row.grade_count -ne '1' -or $row.average -ne '16.5') {
        throw 'CSV reported an incorrect grade count or average.'
    }

    $output = Invoke-StudentManagement @('3', '17', '6', '17', '19', '11', '0')
    Assert-Contains $output 'ID: 17 | Имя: Anna | Оценки: 16.5'
    Assert-Contains $output 'Не удалось экспортировать CSV.'
    $retained = Import-Csv -LiteralPath $report | Where-Object id -eq '17'
    if ($retained.average -ne '16.5') {
        throw 'Export overwrote an existing report.'
    }
    Write-Output 'PASS: grade editing, grade deletion, summary, ordered listing and CSV export'

    $alternate = 'Группа Б.txt'
    $alternateFile = Join-Path $testDirectory.FullName $alternate
    $before = [System.IO.File]::ReadAllText($data)
    $output = Invoke-StudentManagement @('1', '88', 'Лена', '0') -Arguments @('--data', $alternate)
    Assert-Contains $output 'Студент успешно добавлен!'
    $output = Invoke-StudentManagement @('3', '88', '0') -Arguments @('--data', $alternate)
    Assert-Contains $output 'ID: 88 | Имя: Лена | Нет оценок'
    $output = Invoke-StudentManagement @() -ExpectedExit 2 -Arguments @('--data')
    Assert-Contains $output 'Использование:'

    $fileLock = [System.IO.File]::Open("$alternateFile.lock", 'Open', 'ReadWrite', 'None')
    try {
        $output = Invoke-StudentManagement @() -ExpectedExit 1 -Arguments @('--data', $alternate)
        Assert-Contains $output 'Не удалось заблокировать файл данных.'
        $output = Invoke-StudentManagement @('2', '0')
        Assert-Contains $output 'Всего студентов: 4'
    } finally {
        $fileLock.Dispose()
    }
    $output = Invoke-StudentManagement @('3', '88', '0') -Arguments @('--data', $alternate)
    Assert-Contains $output 'ID: 88 | Имя: Лена | Нет оценок'
    if ([System.IO.File]::ReadAllText($data) -cne $before) {
        throw 'Working with an alternate file changed the default data.'
    }
    Write-Output 'PASS: alternate Unicode data path, independent datasets and lock contention'
} finally {
    Get-ChildItem -LiteralPath $testDirectory.FullName -File | Remove-Item
    Remove-Item -LiteralPath $testDirectory.FullName
}

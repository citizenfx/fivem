$Path = $args[0]
$fixedDate = Get-Date -Date "2020-12-01 12:34:56Z"

foreach ($f in (Get-ChildItem -Recurse $Path)) {
    $f.CreationTimeUtc = $fixedDate
    $f.LastWriteTimeUtc = $fixedDate
    $f.LastAccessTimeUtc = $fixedDate
}
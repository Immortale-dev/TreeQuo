param (
    [Int]$count = 1000
)

make rc
$i = $count;
while($i--){
    echo "Test ${i}:"
    Remove-Item ./tmp/t3 -recurse
    ./rc_test > "out.txt"
    $a = Get-Content ./out.txt -tail 1
    echo "${a}";
    if ($a -ne "SUCCEED_") {
        echo "ERROR!!!"
        exit 1;
    }
    echo "OK"
}

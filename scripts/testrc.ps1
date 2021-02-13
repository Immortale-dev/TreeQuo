param (
    [Int]$count = 1000
)

make rc
$i = $count;
while($i--){
    echo "Test ${i}:"
    Remove-Item ./tmp/t3 -recurse -Force -erroraction 'silentlycontinue'
    ./rc_test > "out.txt"
    if ($? -ne 1) {
        echo "ERROR!!!"
        exit 1;
    }
    $a = Get-Content ./out.txt -tail 1
    echo "${a}";
}

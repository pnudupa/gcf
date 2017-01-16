export LD_LIBRARY_PATH=`pwd`/../

# Prepare the results directory
rm -fr Results/Linux 2> /dev/null
mkdir Results 2> /dev/null
cd Results
mkdir Linux
cd Linux
mkdir UnitTests 2> /dev/null
cd ../../

exitCode=0

# Run all the unit-tests first
cd UnitTests
for a in `ls tst_*`
do
        ./$a -o ../Results/Linux/UnitTests/$a.txt
        fCount=$?
        if [ $fCount -eq 0 ]
        then
                printf '  [%4s] : %s\n' $fCount $a
        else
                printf '* [%4s] : %s\n' $fCount $a
                cat ../Results/Linux/UnitTests/$a.txt | grep "^FAIL" -A1
                exitCode=1
        fi
done
cd ..
echo "All test results are stored in ./Results directory"

exit $exitCode

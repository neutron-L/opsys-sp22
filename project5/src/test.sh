#!/bin/bash
pass=0

# Functions
test()
{
    command=$@
    res=$($command | grep result | tail -1 | awk '{print $4}')
    echo $res
    # return $res
}


test-program() {
    for ((i = 4; i <= $2; ++i)) 
    do
        res=$(test ./virtmem $2 $i rand $1)
        if [ $res -eq $3 ]
        then
            echo "test ./virtmem $2 $i rand $1 ... Success"
            pass=$(($pass+1))
        else
            echo -n "test ./virtmem $2 $i rand $1 ... Failed result "
            echo -n $res " "
            echo $3
        fi
        
        res=$(test ./virtmem $2 $i fifo $1)
        if [ $res -eq $3 ]
        then
            echo "test ./virtmem $2 $i fifo $1 ... Success"
            pass=$(($pass+1))
        else
            echo -n "test ./virtmem $2 $i fifo $1 ... Failed result "
            echo -n $res " "
            echo $3
        fi

        res=$(test ./virtmem $2 $i custom $1)
        if [ $res -eq $3 ]
        then
            echo "test ./virtmem $2 $i custom $1 ... Success"
            pass=$(($pass+1))
        else
            echo -n "test ./virtmem $2 $i custom $1 ... Failed result "
            echo -n $res " "
            echo $3
        fi
    done
}

# Main execution
main()
{    
    echo "Test alpha with 10 pages, expect result is 5222400"
    test-program alpha 10 5222400
    echo "Test beta with 10 pages, expect result is 5232896"
    test-program beta  10 5232896
    echo "Test gamma with 10 pages, expect result is 2220835000"
    test-program gamma 10 2220835000
    echo "Test delta with 10 pages, expect result is 5201920"
    test-program delta 10 5201920

    echo "Pass $pass test cases!"
}

main


# vim: sts=4 sw=4 ts=8 ft=sh

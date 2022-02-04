#!/usr/bin/env bash

while getopts "m:n:l:" argv
do
    case $argv in
        m)
            n_host=$OPTARG
            ;;
        n)
            n_players=$OPTARG
            ;;
        l)
            lucky_number=$OPTARG
            ;;
    esac
done

combination=''
declare -a players

for i in $(seq 1 $n_players)
do
    players[$i]=0
done

n_combination=0;

m=$n_players


for a in $(seq 1 $(($m-7)))
do
    for b in $(seq $(($a+1)) $(($m-6)))
    do
        for c in $(seq $(($b+1)) $(($m-5)))
        do
            for d in $(seq $(($c+1)) $(($m-4)))
            do
                for e in $(seq $(($d+1)) $(($m-3)))
                do
                    for f in $(seq $(($e+1)) $(($m-2)))
                    do
                        for g in $(seq $(($f+1)) $(($m-1)))
                        do
                            for h in $(seq $(($g+1)) $m)
                            do
                                combination+="$a $b $c $d $e $f $g $h&"
                                n_combination=$(($n_combination+1))
                                #echo "$a $b $c $d $e $f $g $h"
                            done
                        done
                    done
                done
            done
        done
    done
done



#echo $n_host
#echo $n_players
#echo $lucky_number

IFS='&'
read -ra arr<<<"$combination"




for ((i = 0 ; i <= $n_host ; i++))
do
    #echo ${i}
    fifo_name="fifo_${i}.tmp"
    #echo $fifo_name
    mkfifo $fifo_name
    fd=$(($i+3))
    eval exec "${fd}"'<>"fifo_${i}.tmp"'
done
#echo ${arr[0]}
#t=1
#exec 3< "fifo_0.tmp"
#echo ${arr[0]} > "fifo_${t}.tmp"
#./host -m $t -d 0 -l lucky_number &

t=1
r=1
for i in ${arr[@]}
do
    if [[ $t -le $n_host ]]
    then
        #echo $i
        #echo "fifo_${t}.tmp"
        echo $i > "fifo_${t}.tmp"
        ./host -m $t -d 0 -l $lucky_number &
        #echo "execute host"
    else
        #echo "prepare to read fifo0"
        #exec 3<> "fifo_0.tmp"
        if read -u3 -r line #< "fifo_0.tmp"
        then
            #echo "readline11111"
            #echo "line=${line}"
            #fd=$(($line+3))
            #eval exec "${fd}"'<>"fifo_${line}.tmp"'
            #echo "fifo_${line}.tmp"
            #echo "combination=$i"
            echo $i > "fifo_${line}.tmp"
            for ((i = 0 ; i <= 7 ; i++))
            do
                IFS=' '
                #echo "before read player&score"
                read -u3 player score
                #echo "player=${player} score=${score}"
                players[$player]=$((players[$player]+$score))
            done
            r=$(($r+1))
        fi
        #echo "after read fifo0"
    fi
    t=$(($t+1))
    #echo $t
done

while [ $r -le $n_combination ]
do
    read -u3 -r line
    #echo "backline=$line"
    for ((i = 0 ; i <= 7 ; i++))
    do
        IFS=' '
        #echo "before read player&score"
        read -u3 player score
        #echo "player=${player} score=${score}"
        players[$player]=$((players[$player]+$score))
    done
    r=$(($r+1))
    #echo "-1 -1 -1 -1 -1 -1 -1 -1\n" > "fifo_${line}.tmp"
done

for ((i = 1 ; i <= $n_host ; i++))
do
    #echo ${i}
    #echo "end loooooooooooooooop"
    fifo_name="fifo_${i}.tmp"
    #echo $fifo_name
    echo "-1 -1 -1 -1 -1 -1 -1 -1\n" > $fifo_name
done

#for ((i = 1 ; i <= $n_players ; i++))
#do
#    echo "${i} ${players[$i]}"
#done


for ((i = 1 ; i <= $n_players ; i++))
do
    lose=0
    for ((j = 1 ; j <= $n_players ; j++))
    do
        if [[ $i == $j ]]
        then
            #echo "$i $j"
            continue
        fi
        #echo "${players[$i]} ${players[$j]}"
        if [[ ${players[$i]} -lt ${players[$j]} ]]
        then
            #echo "lose+1"
            lose=$(($lose+1))
        fi
    done
    
    #echo "${lose}"
    rank[$i]=$(($lose+1))
done

for ((i = 1 ; i <= $n_players ; i++))
do
    echo "$i ${rank[$i]}"
done

wait

#echo $n_combination
#for i in $(seq 0 $(($n_combination-1)))
#do
    #echo $i
    #echo ${arr[$i]}
#done

#IFS='&'
#read -ra ARR<<<"$combination"


#t=0
#for i in $(seq 1 $n_host)
#do

    #exec 3<> "fifo_${i}.tmp"
    #echo ${arr[$t]} > "fifo_${i}.tmp"
    
    #echo "before first host"
    #./host -m $i -d 0 -l lucky_number &
    #echo "after first host"
    #t=$(($t+1))
#done

#echo "fifo_0.tmp"


#echo ${arr[$t]}
#echo "1 2 3 4 5 6 7 9\n" > fifo_${line}.tmp
#t=$(($t+1))
#echo ${combination[$t]} > fifo_${line}.tmp
#t=$(($t+1))

#while [ $t -le $n_combination ]
#do
#    echo "startttt"
#    read -u3 -r line
#    echo "endddddd"
#    echo ${combination[$t]}
#    echo ${combination[$t]} > fifo_${line}.tmp
#    t=$(($t+1))
#    for i in $(seq 0 7)
#    do
#        read player score
#        players[$player]=$((players[$player]+$score))
#    done
#done

#while read -u3 line; do
#    echo "read start"
#    echo ${combination[$t]} > fifo_${line}.tmp
#    t=$(($t+1))
#    for i in $(seq 0 7)
#    do
#        read player score
#        players[$player]=$((players[$player]+$score))

#    done
#    echo "read done"
#done < fifo_0.tmp


for ((i = 0 ; i <= $n_host ; i++))
do
    #echo "rm ${i}.tmp"
    rm "fifo_${i}.tmp"
done





            

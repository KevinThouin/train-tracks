#!/bin/sh

P=$(($1))
Q=$(($2))
I=0

printf "$P $(( $P+$P+$Q+$Q ))\n"

while [ $I -lt $(( $P+$P+$Q+$Q )) ]
do
	J=0
	while [ $J -lt $(( $P+$P+$Q+$Q )) ]
	do
		if [ $I -lt $P ] && [ $J -eq $(( $P+$P+$Q-$I-1 )) ]
		then
			printf "01 "
		elif [ $I -lt $(( $P+$Q )) ] && [ $I -ge $P ] && [ $J -eq $(( $P+$P+$P+$Q+$Q-1-$I )) ]
		then
			printf "01 "
		elif [ $I -lt $(( $P+$P+$Q )) ] && [ $I -ge $(( $P+$Q )) ] && [ $J -eq $(( $P+$P+$Q-1-$I )) ]
		then
			printf "10 "
		elif [ $I -ge $(( $P+$P+$Q )) ] && [ $J -eq $(( $P+$P+$P+$Q+$Q-1-$I )) ]
		then
			printf "10 "
		else
			printf "00 "
		fi
		J=$(( $J + 1 ))
	done
	
	printf "\n"
	I=$(( $I + 1 ))
done

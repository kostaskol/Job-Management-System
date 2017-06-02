#!/bin/bash

argc=$#
path="null"
path_p=0
com="null"
com_p=0
max=0
echo -ne "Executing script with arguments:"
#Parse command line arguments
while [[ $# -gt 1 ]]
do
	key="$1"
	case $key in
	-l)
		echo -ne " $1"
		path="$2"
		echo -ne " $path"
		path_p=1
		shift
		;;

	-c)
		echo -ne " $1"
		com="$2"
		echo -ne " $com"
		com_p=1
		shift
		if [[ $argc == 5 ]]; then
			max="$2"
			echo -ne " $max"
			shift
		fi
		;;
esac

shift
done
if [ $path_p == 0 ] || [ $com_p == 0 ] ; then
	echo
	echo "Not all arguments have been provided"
	exit -1
fi
echo
echo

res=($(ls $path))

case $com in
	'list')

	for i in ${res[@]}; do
		echo $i
	done

	;;
	'size')

	res=($(du "--max-depth=1" $path | sort -n -r))
	if [ $max == 0 ] || [ $max -gt ${#res[@]} ]; then
		echo "Printing ${#res[@]} directories in ascending order"
		echo
		for i in $(seq 0 2 ${#res[@]}); do
			echo -ne "${res[i]} "
			p=`expr $i + 1`
			echo ${res[p]}
		done
	else
		echo "Printing $max directories in descending order"
		echo
		tmp=`expr $max - 1`
		len=`expr $tmp \* 2`
		echo 'Size | Name'
		for i in $(seq 0 2 $len); do
			echo -ne "${res[i]} | "
			p=`expr $i + 1`
			echo ${res[p]}
		done
	fi
	;;
	'purge')
	  read -p "Purging path: $path/*. Are you sure? [Y/n]: " yn
		case $yn in
			[Yy]* )
				purge="rm -rf $path/*"
				eval $purge
				;;
			* )
				echo "Exiting with no change"
				;;
			esac
	;;

esac

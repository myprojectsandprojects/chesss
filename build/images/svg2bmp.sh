
if [[ $# != 1 ]]; then
	echo "need 1 argument: directory name"
	exit
fi

# echo "argument provided: $1"
pwd
cd $1
pwd

for f in *; do
	b=${f%.*}
	e=${f#*.}
	# echo "$f $b $e";
	if [[ $e == "svg" ]]
	then
		# echo $f
		convert-im6 -background none -resize 60x60 $f $b.bmp
	else
		echo "not a svg: $f"
	fi
done

# v=123
# echo ${v-novalue}

# v=123456789
# echo ${v:1:4}

# v=abc.123
# echo ${v#*.} # 123
# echo ${v#.*} # abc.123
# echo ${v%.*} # abc
# echo ${v%*.} # abc.123
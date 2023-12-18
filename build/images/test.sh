echo hello world!
for f in *; do
	if [[ -d $f ]]; then
		# echo "$f is a directory"
		bash svg2bmp.sh $f
	fi
done
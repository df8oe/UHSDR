cd ../drivers/freedv
SRCFILES=`grep -L "/* THIS IS A G" *.c *.h`
CODEC2_PATH=../../../../codec2-dev/src
for i in $SRCFILES
do
	XI="$CODEC2_PATH/$i"
	if [ -f "$XI" ]
	then
		cp "$i" "$XI" 			
	else
		echo $i was not found at $XI
	fi
done


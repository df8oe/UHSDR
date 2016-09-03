cd ../drivers/freedv
SRCFILES=`grep -L "/* THIS IS A G" *.c *.h`
CODEC2_PATH=../../../../codec2-dev/src
for i in $SRCFILES
do
	XI="$CODEC2_PATH/$i"
	if [ -f "$XI" ]
	then
		cp $XI $i 			
	else
		echo $i not found 
	fi
done


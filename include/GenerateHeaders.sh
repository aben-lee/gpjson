# Create the ECF headers from scratch
rm -fr gpjson 2> /dev/null
mkdir gpjson

cd ../source
for a in `ls *.h`
do
	cp -v $a ../Include/gpjson/ 
    echo '#include "'$a'"' > ../Include/gpjson/`echo $a | cut -f1 -d.`
done

cd ../cn-cbor/include
for a in `ls *.h`
do
	cp -v $a ../Include/gpjson/ 
    echo '#include "'$a'"' > ../Include/gpjson/`echo $a | cut -f1 -d.`
done

cd ..
cd ../Include/gpjson/
rm -fr *_p
cd ..

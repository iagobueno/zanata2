#!/bin/bash

OUTPUT=sequencial.txt
INPUT=input-edited

if [ -f ${OUTPUT}  ]; then
	rm ${OUTPUT}
fi

echo "Tempo Puramente Sequencial" >> ${OUTPUT} 
echo "" >> ${OUTPUT}
echo "n  | tempo      | resultado " >> ${OUTPUT} 

for i in {13..17}; do

    sed -i "2s/.*/$i/" "${INPUT}"
    ./seq < ${INPUT} >> ${OUTPUT}

done
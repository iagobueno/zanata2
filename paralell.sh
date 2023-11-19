#!/bin/bash

OUTPUT=parallel.txt
INPUT=input-edited

if [ -f ${OUTPUT}  ]; then
	rm ${OUTPUT}
fi

echo "Tempo Paralelo" >> ${OUTPUT} 
echo "" >> ${OUTPUT}
echo "n  | tempo p s | tempo paralelo | resultado " >> ${OUTPUT} 

for j in 1 2 4 8; do
    echo "PROCESSOS: $j" >> ${OUTPUT}
    echo "RODANDO PRA $j"
    for i in {13..17}; do
        sed -i "2s/.*/$i/" "${INPUT}"
        mpirun -np $j ./par < input-edited >> ${OUTPUT}
    done
done

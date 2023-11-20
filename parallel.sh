#!/bin/bash

OUTPUT=parallel.txt
INPUT=input-edited

if [ -f ${OUTPUT}  ]; then
	rm ${OUTPUT}
fi

N=5

for j in 1 2 4 6; do
    echo "NUM DE PROCESSOS = $j" >> ${OUTPUT}
    for i in {13..16}; do
    
        sed -i "2s/.*/$i/" "${INPUT}"
        echo "INSTANCIA = $i" >> ${OUTPUT}

        for seq in {1..2}; do
            echo "RODANDO PRA $j PROCESSO(S), INSTANCIA $i, VEZ $seq"

            TEMP=$(mpirun -np $j ./par < input-edited)

            TIME=$(echo "$TEMP" | cut -d'|' -f3)
            numeric_value=$(echo "$TIME" | awk '{gsub(/[^0-9.]/,""); print}')

            # Append the values to the arrays
            TIME_ARRAY+=("$TIME")
            numeric_values+=("$numeric_value")
        done

        # Calculate the total and count for the average
        total=0
        count=${#numeric_values[@]}

        
        # Loop through the array to calculate the total
        for value in "${numeric_values[@]}"; do
            total=$(awk "BEGIN {print $total + $value}")
        done

        # Calculate the average
        average=$(awk "BEGIN {print $total / $count}")

        # Calculate the sum of squared differences for standard deviation
        sum_squared_diff=0
        for value in "${numeric_values[@]}"; do
            diff=$(awk "BEGIN {print $value - $average}")
            squared_diff=$(awk "BEGIN {print $diff * $diff}")
            sum_squared_diff=$(awk "BEGIN {print $sum_squared_diff + $squared_diff}")
        done

        # Calculate the standard deviation
        std_deviation=$(awk "BEGIN {print sqrt($sum_squared_diff / $count)}")

        echo "Average: $average" >> ${OUTPUT}
        echo "Standard Deviation: $std_deviation" >> ${OUTPUT}
        echo >> ${OUTPUT}
    done
done

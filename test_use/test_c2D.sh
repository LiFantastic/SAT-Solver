#!/bin/sh

timeout() {

    time=$1

    # start the command in a subshell to avoid problem with pipes
    # (spawn accepts one command)
    command="/bin/sh -c \"$2\""

    expect -c "set echo \"-noecho\"; set timeout $time; spawn -noecho $command; expect timeout { exit 1 } eof { exit 0 }"    

    if [ $? = 1 ] ; then
        echo "Timeout after ${time} seconds"
        return 0
    else
    		return 1
    fi
}

DATAPATH="../benchmarks/sampled/"
#RESULTPATH="./c2D_result/_p_0_1stUIP/"
RESULTPATH="./c2D_result/_p_0_provided/"

MYC2D=./mine/c2D
PROVIDED=./darwin/c2D

DATAGEN=./get_data.py

TOTEST=${PROVIDED}

vtree_type="p"
vtree_method="0"

CNF_FILES=`find ${DATAPATH} -name '*.cnf' -exec basename {} \;`

#JAVA_FILES=`find data -name '*.cnf' -print`

# SAT=$(echo ${CNF_FILES} | grep "s")

# if [ -z "${SAT}" ]
# then
# 	for FILE in ${SAT}
# do
# 	echo ${FILE}
# 	./sat -in_cnf ${FILE}
# done
rm ${RESULTPATH}*.csv
for FILE in ${CNF_FILES}
do
	if [ ! -f "${RESULTPATH}${FILE}.txt" ]; then
		echo "Processing "${FILE}
		timeout 36000 "${TOTEST} --vtree_type ${vtree_type} --vtree_method ${vtree_method} -C -E -c ${DATAPATH}${FILE} > ${RESULTPATH}${FILE}.tmp"
		if [ $? -eq 0 ] ; then
			rm ${RESULTPATH}${FILE}.tmp
			continue
		else
			cat ${RESULTPATH}${FILE}.tmp > ${RESULTPATH}${FILE}.txt
			rm ${RESULTPATH}${FILE}.tmp
		fi
		rm ${DATAPATH}*.nnf
	else
		echo ${FILE}" Already Tested"
	fi
	python ${DATAGEN} ${RESULTPATH}${FILE}.txt ${RESULTPATH}"result.csv"
done
rm *.tmp


#rm SAT_CASE.txt

#!/bin/sh

timeout() {

    time=$1

    # start the command in a subshell to avoid problem with pipes
    # (spawn accepts one command)
    command="/bin/sh -c \"$2\""

    expect -c "set echo \"-noecho\"; set timeout $time; spawn -noecho $command; expect timeout { exit 1 } eof { exit 0 }"    

    if [ $? = 1 ] ; then
        echo "Timeout after ${time} seconds"
    fi

}


DATAPATH="../extra_data/"
RESULTPATH="./sat_result/"

CNF_FILES=`find ${DATAPATH} -name '*.cnf' -exec basename {} \;`

MYSAT=./mine/sat
PROVIDED=./darwin/sat

#JAVA_FILES=`find data -name '*.cnf' -print`

# SAT=$(echo ${CNF_FILES} | grep "s")

# if [ -z "${SAT}" ]
# then
# 	for FILE in ${SAT}
# do
# 	echo ${FILE}
# 	./sat -in_cnf ${FILE}
# done

#rm SAT_CASE.txt

# for FILE in ${CNF_FILES}
# do
# 	if [ ${FILE:14:1} = "s" ];then
# 		echo ${FILE:5:14}"    \c"  >> SAT_CASE.txt
# 		echo `../sat_solver/sat -c ${FILE}` >> SAT_CASE.txt
# 	fi
# 	#./sat -in_cnf $FILE
# done

echo " " > ${RESULTPATH}myResult.txt

for FILE in ${CNF_FILES}
do
	echo ${FILE}"\n" >> ${RESULTPATH}myResult.txt
	
	(time ${MYSAT} -c ${DATAPATH}${FILE}) >& ${RESULTPATH}${FILE}.tmp
	cat ${RESULTPATH}${FILE}.tmp >> ${RESULTPATH}myResult.txt

	# echo ${FILE}"\n" >> ${RESULTPATH}givenResult.txt
	# (time ${PROVIDED} -c ${DATAPATH}${FILE}) &> ${RESULTPATH}${FILE}.tmp
	# cat ${RESULTPATH}${FILE}.tmp >> ${RESULTPATH}givenResult.txt

	# rm ${RESULTPATH}${FILE}.tmp
done




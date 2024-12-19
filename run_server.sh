
APP=${1}
SERVER=${2}
WUP_REQ=${3}
MAX_REQ=${4}
THREADS=${5}

cd ${APP}/
./run_networked_server.sh ${SERVER} ${WUP_REQ} ${MAX_REQ} ${THREADS}




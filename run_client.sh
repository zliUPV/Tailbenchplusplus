

APP=${1}
SERVER=${2}
SERVER_PORT=${3}
QPS=${4}
THREADS=${5}

cd ${APP}/

./run_networked_client.sh ${SERVER} ${SERVER_PORT} ${QPS} ${THREADS}

../utilities/parselats.py lats.bin

rm lats.bin


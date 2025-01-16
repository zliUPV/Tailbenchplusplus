# Tailbench++
It is an expanded version of Tailbench, which does not restrict the number of clients before hand. Therefore, the server will treat an undefined number of clients. 
All the modifications are only applicable to the Networked version. 

## Installation process
This section describe the process to compile Tailbench++. 

### Dependencies
```bash
  sudo apt-get install build-essential make gcc g++ pkg-config libopencv-dev libboost-all-dev libgoogle-perftools-dev libssl-dev ant uuid-dev
  sudo apt-get install bison swig openjdk-8-jdk-headless libnuma-dev libdb5.3++-dev libmysqld-dev libaio-dev libgtop2-dev libreadline-dev libncurses5-dev libncursesw5-dev
  sudo apt-get install autoconf python3.9 python3-pip libjemalloc-dev
  sudo apt-get install automake zlib1g-dev cmake wget libnsl-dev  
  pip install numpy scipy
```
### Building
Tailbench++ code can be divided into 9 parts in their own directories:
- harness 
- img-dnn
- masstree
- moses
- shore 
- silo
- specjbb
- sphinx
- xapian 

Each part has each own `build.sh` script, which does all the compilation. However, some parts need additional steps before executing the script. 

#### Configuration
In Tailbenchplusplus directory, there is a file named `configs.sh` which need to be modified to specify the location of the data directory, the JDK_PATH, and scratch directory to save temporary files during execution of some applications. 
```bash
 DATA_ROOT=/PATH/TO/TAILBENCH/DATASET/FOLDER #location of tailbench.inputs directory
 JDK_PATH=/PATH/TO/JAVA/JDK #normally /usr/lib/jvm/java-8-openjdk-amd64/
 SCRATCH_DIR=/PATH/TO/SCRATCH/FOLDER #the directory could be any folder you create

```
Another file `Makefile.config`, which is also located on the main directory, should have the JDK_PATH.

Theses configurations should be done before compiling the code. 

#### harness
It is the part that must be compile before the others the others applications. 

#### masstree
if a problem arises because of not finding `config.h` file:
```bash
./configure 
```
#### sphinx
Before executing `build.sh` scripts, the following steps should be made.  

1. Compilation of sphinxbase library: 
```bash
# compile sphinxbase 
tar -xvf sphinxbase-5prealpha.tar.gz 
cd sphinxbase-5prealpha
sudo ./autogen.sh 
sudo ./configure 
sudo make -j$(nproc)
sudo make install
```
2. Compilation of pocketsphinx library
```bash
tar -xvf pocketsphinx-5prealpha.tar.gz 
cd pocketsphinx-5prealpha
sudo ./autogen.sh 
sudo ./configure 
sudo make -j$(nproc)
sudo make install 
```
#### xapian 
Before executing `build.sh` script, the following steps should be made. 
```bash
tar -xvf xapian-core-1.2.13 
cd xapian-core-1.2.13 
sudo ./configure 
sed -i 's/CXX = g++/CXX = g++ -std=c++03/g' Makefile 
sudo make -j$(nproc)
sudo make install
```
### Input files and Data 
Download the dataset used by the Tailbenchplusplus from [Tailbench Dataset](https://tailbench.csail.mit.edu/tailbench.inputs.tgz). 
The location of the data should be specified on the `configs.sh` as the DATA_ROOT variable. 
```bash
wget https://tailbench.csail.mit.edu/tailbench.inputs.tgz
tar tailbench.inputs.tgz -C DESTINATION
```
## Environment variables

**TBENCH_WARMUPREQS**: This variable specifies the number of requests during the warmup. During the warmup time, latency is not measured. The variable have to be passed to the client module. 

**TBENCH_MAXREQS**: It represents the number of requests to be execute during measurement time. It must be passed to the client module when it is executed. 

**TBENCH_MINSLEEPNS**: It is the minimum length of tiem, in ns, for which the client sleeps in the kernel upon encountering an idle period. 

**TBENCH_RANDSEED**: Seed for the random number generator that generates interarrival times.

**TBENCH_CLIENT_THREADS**: The numbers of threads used by the client to generate the requests. Although the request rate is defined by **TBENCH_QPS**, this variable is useful when a single client cannot reach to the rate specified. 

**TBENCH_SERVER**: This variable specifies the URL or IP of the server. Default value is localhost.

**TBENCH_SERVER_PORT**: It is the port through which the server listens. Default value 8080.

**TBENCH_QPS**: It represents the average request per second. It must be passed to the client module. 

**TBENCH_VARQPS**: This variable could have 3 values (0, 1 and 2). When the variable is not 0, the client will vary it request rate dinamically during execution. When 1, variables **TBENCH_INIQPS** and **TBENCH_STEPQPS** must be specified. When 2, variables **TBENCH_INIQPS**, **TBENCH_INTERVALQPS**, **TBENCH_QPS_1**, **TBENCH_QPS_2**, **TBENCH_QPS_3** and **TBENCH_QPS_4** must be specified. 

**TBENCH_STEPQPS**: This parameter represents the number of queries that the rate of requests will increase after each request. It is only effective when **TBENCH_VARQPS** == 1. 

**TBENCH_INTERVALQPS**: This variable indicates how many requests the client will vary the rate of requests. In other words, it indicates the period of time during which the QPS is stable. The variable is proportional to the actual rate. We only need to specify a first value for **TBENCH_INIQPS**, when the rate is changed during execution, the interval is recalculated. 

**TBENCH_INIQPS**: It is the initial rate of queries, when client is configure to vary its rate of requests during execution. 

**TBENCH_QPS_X**(1,2,3,4): This variables represents additional rates of requests when **TBENCH_VARQPS**==2. The client starts with **TBENCH_INIQPS**, after completing **TBENCH_INTERVALQPS** queries, it will change to **TBENCH_QPS_1** and **TBENCH_INTERVALQPS** is updated to make the client stay on this rate the same time as the previous request rate. The client cyclically loops through TBENCH_INIQPS and TBENCH_QPS_X values. TBENCH_INIQPS -> TBENCH_QPS_1 -> TBENCH_QPS_2 -> TBENCH_QPS_3 -> TBENCH_QPS_4 -> TBENCH_INIQPS -> TBENCH_QPS_1 .....


## How to run 

On each application directory there 2 example scripts, `run_networked_server.sh` and `run_networked_client.sh`, which show how to run each application.

To run a client that can vary its request rate, here you have an example with `xapian` in the two types of request rate variation:
```bash
#!/bin/bash 

TBENCH_SERVER=127.0.0.1 TBENCH_SERVER_PORT=8080 TBENCH_WARMUPREQS=1000 TBENCH_MAXREQS=20000 \
  TBENCH_VARQPS=2 TBENCH_INIQPS=100 TBENCH_INTERVALQPS=1000 TBENCH_QPS_1=200 TBENCH_QPS_2=400 TBENCH_QPS_3=500 \
  TBENCH_QPS_4=600 TBENCH_CLIENT_THREADS=1 TBENCH_MINSLEEPNS=100000 TBENCH_TERMS_FILE=${DATA_ROOT}/xapian/terms.in \
  ./xapian_networked_client & 

TBENCH_SERVER=127.0.0.1 TBENCH_SERVER_PORT=8080 TBENCH_WARMUPREQS=1000 TBENCH_MAXREQS=20000 \
  TBENCH_VARQPS=1 TBENCH_INIQPS=100 TBENCH_STEPQPS=5 TBENCH_CLIENT_THREADS=1 TBENCH_MINSLEEPNS=100000 \
  TBENCH_TERMS_FILE=${DATA_ROOT}/xapian/terms.in \
  ./xapian_networked_client & 

```

*Note*: 
- if you want to run shore client and server localhost, use `run_networked_client_loopback.sh` to initialize clients. 

## Additional notes
There are two main branches, `main` and `branch_to_ubuntu24`. The main branch functions on Ubuntu 18, while the second branch is an modification of the first one to adapt the benchmark suite to be compilable on newer operating systems. So far, we have succeed in compiling 7 out of 8 applications, which are xapian, img-dnn, masstree, shore, sphinx, silo and specjbb.

It is recommended to compile each part one by one, though there is a script `build.sh` on the main directory which call the compilation script of each part. 


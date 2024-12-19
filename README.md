# tailbench
tailbench improved and expanded 


**INSTALLATION**

*Input files:*

    - Download from: .... ToDo

*Dependencies:*

    - sudo apt-get install make g++ pkg-config libopencv-dev libgoogle-perftools-dev libssl-dev ant uuid-dev bison swig openjdk-8-jdk-headless libnuma-dev libdb5.3++-dev libmysqld-dev libaio-dev libgtop2-dev libreadline-dev libncurses5-dev libncursesw5-dev

    - sudo apt install autoconf python python-dev python-pip

    - pip install numpy scipy

    - sudo apt-get install libjemalloc-dev 


*Configuration:*

    tar -xzvf tailbench-v0.9.tgz 

    tar -xzvf tailbench.inputs.tgz

    cd tailbench-v0.9/

    In configs.sh:

        set JDK_PATH to /usr/lib/jvm/java-8-openjdk-amd64

        set DATA_ROOT to /home/jofepre/tailbench.inputs

    In Makefile.config:

        set JDK_PATH to /usr/lib/jvm/java-8-openjdk-amd64

    
    ./build.sh harness
    
    ./build.sh *application*

    FOR XAPIAN FIRST:
 
       cd /xapian-core-1.2.13 
       make -B

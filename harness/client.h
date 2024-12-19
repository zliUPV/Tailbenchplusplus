/** $lic$
 * Copyright (C) 2016-2017 by Massachusetts Institute of Technology
 *
 * This file is part of TailBench.
 *
 * If you use this software in your research, we request that you reference the
 * TaiBench paper ("TailBench: A Benchmark Suite and Evaluation Methodology for
 * Latency-Critical Applications", Kasture and Sanchez, IISWC-2016) as the
 * source in any publications that use this software, and that you send us a
 * citation of your work.
 *
 * TailBench is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 */

#ifndef __CLIENT_H
#define __CLIENT_H

#include "msgs.h"
#include "msgs.h"
#include "dist.h"

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>

#include <atomic>
#include <map>

enum ClientStatus { INIT, WARMUP, ROI, FINISHED };

class Client {
protected:
    ClientStatus status;

    int nthreads;
    int client_id;
    // CLIENT CONTROLS QUERIES //
    uint64_t warmupreqs;
    uint64_t maxreqs;
    std::atomic<uint64_t> numReqsSent; //increments 1 in each successful send
    std::atomic<uint64_t> numReqsCompleted; //increments 1 in each successful response
    // --------------------- //
    pthread_mutex_t lock;
    pthread_barrier_t barrier;

    uint64_t minSleepNs;
    uint64_t seed;
	// ADDED TO VARIABLE QPS //
	int qpsVar;	//1-> Use variable QPS 0-> Do not use variable QPS
	double qpsIni;	// START QPS
	int qpsInterval;	// INTERVAL QPS
	double qpsStep;	// STEPS
	int qpsCounter;
    // MODIFIED VAR QPS //
    std::vector<double> listQPS; // save QPS different qps values
    size_t idxListQPS;
	// --------------------- //
    double lambda;
    ExpDist* dist;

    uint64_t startedReqs;
    std::unordered_map<uint64_t, Request*> inFlightReqs;

    std::vector<uint64_t> svcTimes;
    std::vector<uint64_t> queueTimes;
    std::vector<uint64_t> sjrnTimes;
    std::vector<uint64_t> requestDate;
    std::vector<uint64_t> NicSjrnTimes; // NIC to NIC latency (from send to receive)

    uint64_t gen_requests;
    uint64_t timely_gen_requests;
    uint64_t sent_requests;

    uint64_t gen_req_ns;
    uint64_t send_req_ns;

    uint64_t total_req_ns;
    uint64_t total_sleep_ns; 

    uint64_t roi_start_ns;
    int qps_error;

    void _startRoi();

public:
    Client(int nthreads, int client_id, uint64_t warmupreq, uint64_t maxreq);

    Request* startReq();
    void finiReq(Response* resp);

    void startRoi();
    void dumpStats();

    // CLIENT CONTROLS QUERIES //
    uint64_t get_warmupreq() const;
    uint64_t get_maxreq() const;
    const std::atomic<uint64_t>& get_reqsSent();
    const std::atomic<uint64_t>& get_reqsCompleted();
    // --------------------- //

};

class NetworkedClient : public Client {
private:
    pthread_mutex_t sendLock;
    pthread_mutex_t recvLock;

    int serverFd;
    std::string error;

public:
    NetworkedClient(int nthreads, std::string serverip, int serverport, int client_id, uint64_t warmupreq, uint64_t maxreq);
    bool send(Request* req);
    bool recv(Response* resp);
    const std::string& errmsg() const { return error; }

};

#endif

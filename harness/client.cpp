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

#include "client.h"
#include "helpers.h"
#include "tbench_client.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <chrono>
#include <ctime>
#include <sys/time.h>

#include <inttypes.h>

/*******************************************************************************
 * Client
 *******************************************************************************/

Client::Client(int _nthreads, int _client_id, uint64_t warmupreq, uint64_t maxreq)
{
    status = INIT;

    nthreads = _nthreads;
    client_id = _client_id;
    pthread_mutex_init(&lock, nullptr);
    pthread_barrier_init(&barrier, nullptr, nthreads);

    minSleepNs = getOpt("TBENCH_MINSLEEPNS", 0);
    seed = getOpt("TBENCH_RANDSEED", 0) + client_id;
    lambda = getOpt<double>("TBENCH_QPS", 1000.0) * 1e-9;

    qpsVar = getOpt("TBENCH_VARQPS", 0);

    if (qpsVar == 1)
    {
        fprintf(stderr, "\nUsing variable QPS!\n");
        qpsIni = getOpt<double>("TBENCH_INIQPS", 1000.0) * 1e-9;
        qpsInterval = getOpt("TBENCH_INTERVALQPS", 0);
        qpsStep = getOpt<double>("TBENCH_STEPQPS", 1000.0) * 1e-9;


    } else if(qpsVar == 2) {
        // VARIABLE QPS v2
        fprintf(stderr, "\nUsing variable QPS v2!\n");
        qpsIni = getOpt<double>("TBENCH_INIQPS", 1000.0) * 1e-9;
        qpsInterval = getOpt("TBENCH_INTERVALQPS", 0);
        listQPS.push_back(qpsIni);
        double auxQPS = getOpt<double>("TBENCH_QPS_1", 1000.0) * 1e-9;
        listQPS.push_back(auxQPS);
        auxQPS = getOpt<double>("TBENCH_QPS_2", 1000.0) * 1e-9; 
        listQPS.push_back(auxQPS);
        auxQPS = getOpt<double>("TBENCH_QPS_3", 1000.0) * 1e-9;
        listQPS.push_back(auxQPS);
        auxQPS = getOpt<double>("TBENCH_QPS_4", 1000.0) * 1e-9;
        listQPS.push_back(auxQPS);
        idxListQPS = 1;
    }

    dist = nullptr; // Will get initialized in startReq()

    startedReqs = 0;

    gen_requests = 0;
    timely_gen_requests = 0;
    sent_requests = 0;

    gen_req_ns = 0;
    send_req_ns = 0;
    total_req_ns = 0;
    total_sleep_ns = 0;

    // CLIENT CONTROLS QUERIES //
    maxreqs = maxreq;
    warmupreqs = warmupreq;
    numReqsSent = 0;
    numReqsCompleted = 0;
    // ----------------------- //

    qps_error = 0;

    tBenchClientInit();
}

Request *Client::startReq()
{

    // std::cerr << "   [startReq] start" << std::endl;

    uint64_t startNs = getCurNs();

    if (status == INIT)
    {

        // std::cerr << "   [startReq] init" << std::endl;

        pthread_barrier_wait(&barrier); // Wait for all threads to start up

        // std::cerr << "   [startReq] all threads ready" << std::endl;

        pthread_mutex_lock(&lock);

        // std::cerr << "   [startReq] mutex locked" << std::endl;

        if (!dist)
        {

            uint64_t curNs = getCurNs();
            if (!qpsVar)
            {
                dist = new ExpDist(lambda, seed, curNs);
            }
            else
            {
                dist = new ExpDist(qpsIni, seed, curNs);
            }
            status = WARMUP;

            pthread_barrier_destroy(&barrier);
            pthread_barrier_init(&barrier, nullptr, nthreads);
        }

        pthread_mutex_unlock(&lock);
        // std::cerr << "   [startReq] mutex locked" << std::endl;
        pthread_barrier_wait(&barrier);
        // std::cerr << "   [startReq] init done" << std::endl;
    }

    
    if (status == ROI && qpsVar)
    {
        // COMPROBAR  E INCREMENTAR QPS
        // fprintf(stderr, "\nSTARTED ROI... REQ:%" PRIu64 "\t COUNTER:%d \t INTERVAL:%d\n", startedReqs, qpsCounter, qpsInterval);
        pthread_mutex_lock(&lock);
        if (qpsCounter == qpsInterval)
        {
            qpsCounter = 0;

            if(qpsVar == 1) {
                qpsIni += qpsStep; // 10 ---- 100 
            } else if(qpsVar == 2) {
                // var qps v2 10 20 30 40 50
                double newqps = listQPS[idxListQPS%listQPS.size()];
                double ratio = newqps/qpsIni;
                //fprintf(stderr, "\nQPSnew changed to %f\n", newqps);
                //fprintf(stderr, "\nQPSration changed to %f\n", ratio);
                //fprintf(stderr, "\nQPSoriginalinterval %f\n", qpsInterval);
                if(newqps < qpsIni) qpsInterval = static_cast<int>(qpsInterval*ratio) + 1;
                else qpsInterval = (qpsInterval*ratio); //interval is proportional to qps
                //double ratio = round(list)
                qpsIni = newqps;
                idxListQPS++;
                
            }
            
            dist = new ExpDist(qpsIni, seed, getCurNs());
            fprintf(stderr, "\nQPS changed to %f\n", qpsIni / 1e-9);
            fprintf(stderr, "\nQPS interval changed to %d\n", qpsInterval);
        }
        else
        {
            qpsCounter++;
        }
        pthread_mutex_unlock(&lock);
    }
    

    // std::cerr << "   [startReq] creating request " << std::endl;
    Request *req = new Request(); // It can be placed before the lock as it is independent of the threads and takes some not-negligible time
    // std::cerr << "   [startReq] done creating " << std::endl;
    req->startNs = startNs;

    pthread_mutex_lock(&lock);

    req->startNs_lock_ad = getCurNs(); // Time when the lock is adquired

    // std::cerr << "   [startReq] generating request " << std::endl;
    size_t len = tBenchClientGenReq(&req->data);
    req->len = len;

    // std::cerr << "   [startReq] done generating " << std::endl;

    req->id = startedReqs++;
    req->genNs = dist->nextArrivalNs();
    inFlightReqs[req->id] = req;

    // std::cerr << "   [startReq] obtained generation time " << std::endl;

    // Should be done inside the lock
    gen_requests++;
    if (req->genNs > req->startNs)
    {
        timely_gen_requests++;
    }

    req->startNs_lock_re = getCurNs(); // Time when the lock is released
    gen_req_ns += req->startNs_lock_re - req->startNs_lock_ad;

    pthread_mutex_unlock(&lock);

    if (req->genNs < req->startNs)
    {
        req->late = true;
    }
    else
    {
        req->late = false;
    }

    /////////////////////////////////////////////////////////////////
    /*if (qps_error == 0 && status == ROI && (gen_requests > 1000) && (((unsigned long long) timely_gen_requests * 1.0) / ((unsigned long long) gen_requests * 1.0) < 0.95)) {
        qps_error = 1;
        fprintf(stderr, "\n\nERROR!! The client cannot reach the requested QPS (%.2f)! Please, increase the number of threads and/or clients.\n", lambda * 1000000000);
        fprintf(stderr, "# requests: %llu  # timely requests: %llu  Percentatge timely requests: %.3f. The percentatge is below the current threshold (0.95)\n", (unsigned long long) gen_requests, (unsigned long long) timely_gen_requests, ((unsigned long long) timely_gen_requests * 1.0) / ((unsigned long long) gen_requests * 1.0));
        fprintf(stderr, "Effective QPS: %.2f\n\n\n", (double) timely_gen_requests / (((unsigned long long) getCurNs() - (unsigned long long) roi_start_ns) / (double) 1000000000));
    }*/
    /////////////////////////////////////////////////////////////////

    uint64_t curNs = getCurNs();
    if (curNs < req->genNs)
    {
        sleepUntil(std::max(req->genNs, curNs + minSleepNs));
        total_sleep_ns += std::max(req->genNs - curNs, minSleepNs);
    }

    // std::cerr << "   [startReq] going to return request " << std::endl;
    return req;
}

void Client::finiReq(Response *resp)
{

    pthread_mutex_lock(&lock);

    auto it = inFlightReqs.find(resp->id);
    assert(it != inFlightReqs.end());

    Request *req = it->second;

    uint64_t curNs = getCurNs();
    assert(curNs > req->genNs);

    uint64_t sjrn, qtime, NicSjrn;

    if (!req->late)
    {

        sjrn = curNs - req->genNs;

        if (resp->svcNs > sjrn)
        {
            fprintf(stderr, "WARNING: next assert would have failed. Req->id: %llu Req->startNs: %llu Req->genNs: %llu Req->sentNs: %llu -- CurNs: %llu ----- Resp->id: -- %llu Resp->svcNs: %llu \n",
                    (unsigned long long)req->id,
                    (unsigned long long)req->startNs,
                    (unsigned long long)req->genNs,
                    (unsigned long long)req->sentNs,
                    (unsigned long long)curNs,
                    (unsigned long long)resp->id,
                    (unsigned long long)resp->svcNs);

            qtime = 0;
        }

        else
        { // Cas normal: sjrn > resp->svcNs

            // assert(sjrn >= resp->svcNs);
            qtime = sjrn - resp->svcNs;
        }
    }
    else
    {
        // If the request was not timely, assume it as generated in startNs
        // This might introduce some innacuracy, so we should make sure that the percentatge of non-timely requests is low
        sjrn = curNs - req->startNs;

        if (resp->svcNs > sjrn)
        {
            fprintf(stderr, "WARNING: next assert would have failed (non-timely request). Req->id: %llu Req->startNs: %llu Req->genNs: %llu Req->sentNs: %llu -- CurNs: %llu ----- Resp->id: -- %llu Resp->svcNs: %llu \n",
                    (unsigned long long)req->id,
                    (unsigned long long)req->startNs,
                    (unsigned long long)req->genNs,
                    (unsigned long long)req->sentNs,
                    (unsigned long long)curNs,
                    (unsigned long long)resp->id,
                    (unsigned long long)resp->svcNs);

            qtime = 0;
        }

        else
        {
            // assert(sjrn >= resp->svcNs);
            qtime = sjrn - resp->svcNs;
        }
    }

    NicSjrn = curNs - req->sentNs_lock_ad;

    if (status == ROI)
    {

        // PRINT TIME
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long millis = (unsigned long long)(tv.tv_usec % 1000000);

        fprintf(stderr, "TIME:%.19s.%llu\tRequest %" PRIu64 " :\tQueueTime: %.3f\t ServiceTime: %.3f\t SojournTime: %.3f \t NicSojournTime: %.3f \t SendingTime: %.3f \t ReceivingTime: %.3f\n",
                ctime(&timenow), millis, resp->id,
                (unsigned long long)qtime / (double)1000000, // from ns to ms
                (unsigned long long)resp->svcNs / (double)1000000,
                (unsigned long long)sjrn / (double)1000000,
                (unsigned long long)NicSjrn / (double)1000000,
                (unsigned long long)(req->sentNs_lock_re - req->sentNs_lock_ad) / (double)1000000,
                (unsigned long long)(resp->recvNs) / (double)1000000);

        // uint64_t t_micros = (tv.tv_sec * (int)1e6) + tv.tv_usec;
        // requestDate.push_back(t_micros);

        if (!req->late)
        {
            requestDate.push_back(req->genNs);
        }
        else
        {
            requestDate.push_back(req->startNs);
        }

        queueTimes.push_back(qtime);
        svcTimes.push_back(resp->svcNs);
        sjrnTimes.push_back(sjrn);
        NicSjrnTimes.push_back(NicSjrn);

        // pid_t x = syscall(__NR_gettid);
    }

    delete req;
    inFlightReqs.erase(it);

    // CLIENT CONTROLS QUERIS //
    // if the client completes the number of requests, terminate the
    ++numReqsCompleted;
    if (numReqsCompleted == warmupreqs || warmupreqs== 0)
    {
        //fprintf(stderr, "Warmup requests (%llu) completed. Send ROI begin!\n", (unsigned long long) warmupreqs);
        _startRoi();
    }
    else if (numReqsCompleted >= (warmupreqs + maxreqs))
    {
        //fprintf(stderr, "Overall number of requests (%llu) completed. Send FINISH begin!\n", (unsigned long long) numReqsCompleted);
        dumpStats();
        fprintf(stderr, "--> Client archiving total requests FINISH\n");
        pthread_mutex_unlock(&lock);
        syscall(SYS_exit_group, 0);
    }
    // ----------------------------------------------------- //
    
    pthread_mutex_unlock(&lock);
}

void Client::_startRoi()
{
    assert(status == WARMUP);
    status = ROI;

    fprintf(stderr, "--> Client %d -- ROI BEGIN\n", client_id);

    // Cuando Acaba WARMUP
    qpsCounter = 0;
    requestDate.clear();
    queueTimes.clear();
    svcTimes.clear();
    sjrnTimes.clear();
    NicSjrnTimes.clear();

    gen_requests = 0;
    timely_gen_requests = 0;
    sent_requests = 0;
    gen_req_ns = 0;
    send_req_ns = 0;
    total_req_ns = 0;
    total_sleep_ns = 0;

    roi_start_ns = getCurNs();
}

void Client::startRoi()
{
    pthread_mutex_lock(&lock);
    _startRoi();
    pthread_mutex_unlock(&lock);
}

void Client::dumpStats()
{
    uint64_t fiNs = getCurNs();

    std::string file = std::string("lats_" + std::to_string(client_id) + ".bin");
    std::ofstream out(file, std::ios::out | std::ios::binary);
    int reqs = sjrnTimes.size();

    for (int r = 0; r < reqs; ++r)
    {
        out.write(reinterpret_cast<const char *>(&queueTimes[r]),
                  sizeof(queueTimes[r]));
        out.write(reinterpret_cast<const char *>(&svcTimes[r]),
                  sizeof(svcTimes[r]));
        out.write(reinterpret_cast<const char *>(&sjrnTimes[r]),
                  sizeof(sjrnTimes[r]));
        out.write(reinterpret_cast<const char *>(&requestDate[r]),
                  sizeof(requestDate[r]));
        out.write(reinterpret_cast<const char *>(&NicSjrnTimes[r]),
                  sizeof(NicSjrnTimes[r]));
    }
    out.close();

    // CLIENT DECIDES QUERIES //
    gen_requests = numReqsCompleted - warmupreqs;
    // ---------------------- //

    double perc_timely_req = ((unsigned long long)timely_gen_requests * 1.0) / ((unsigned long long)gen_requests * 1.0);
    fprintf(stderr, "# gen requests: %llu  # timely gen requests: %llu  Percentatge timely gen requests: %.3f\n",
            (unsigned long long)gen_requests, (unsigned long long)timely_gen_requests, perc_timely_req);
    if (perc_timely_req < 0.975)
    {
        fprintf(stderr, "\n##### REQUEST DISTRIBUTION BROKEN #####\n\n");
    }

    fprintf(stderr, "Requested QPS: %.2f Effective QPS: %.2f  --  Start: %.3f End: %.3f\n\n\n", lambda * 1000000000, (double)sent_requests / (((unsigned long long)fiNs - (unsigned long long)roi_start_ns) / (double)1000000000),
            (unsigned long long)roi_start_ns / (double)1000000, (unsigned long long)fiNs / (double)1000000);

    // fprintf(stderr, "Total_request_time (ms): %.3f -- Total_sleep_time (ms): %.3f\n", (unsigned long long) total_req_ns / (double) 1000000 / gen_requests, (unsigned long long) total_sleep_ns / (double) 1000000 / gen_requests);

    // fprintf(stderr, "Request_generation_time (ms): %.3f  Request_send_time (ms): %.3f\n", (unsigned long long) gen_req_ns / (double) 1000000 / gen_requests, (unsigned long long) send_req_ns / (double) 1000000 / gen_requests);

    // if ((gen_requests > 1000) && (((unsigned long long) timely_gen_requests * 1.0) / ((unsigned long long) gen_requests * 1.0) < 0.95)) {
    //     fprintf(stderr, "\n\n\nWARNING!! Couldn't reach requested QPS! (Generation time of requests updated at send).\n");

    //    fprintf(stderr, "# requests: %llu  # timely requests: %llu  Percentatge timely requests: %.3f. The percentatge is below the current threshold (0.95)\n", (unsigned long long) gen_requests, (unsigned long long) timely_gen_requests, ((unsigned long long) timely_gen_requests * 1.0) / ((unsigned long long) gen_requests * 1.0));
    //}
}

// CLIENT CONTROLS QUERIES //
uint64_t Client::get_warmupreq() const
{
    return warmupreqs;
}

uint64_t Client::get_maxreq() const
{
    return maxreqs;
}

const std::atomic<uint64_t> &Client::get_reqsSent()
{
    return numReqsSent;
}

const std::atomic<uint64_t> &Client::get_reqsCompleted()
{
    return numReqsCompleted;
}

/*******************************************************************************
 * Networked Client
 *******************************************************************************/
NetworkedClient::NetworkedClient(int nthreads, std::string serverip,
                                 int serverport, int client_id, uint64_t warmupreq, uint64_t maxreq) : Client(nthreads, client_id, warmupreq, maxreq)
{
    pthread_mutex_init(&sendLock, nullptr);
    pthread_mutex_init(&recvLock, nullptr);

    // Get address info
    int status;
    struct addrinfo hints;
    struct addrinfo *servInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream portstr;
    portstr << serverport;

    const char *serverStr = serverip.size() ? serverip.c_str() : nullptr;

    if ((status = getaddrinfo(serverStr, portstr.str().c_str(), &hints, &servInfo)) != 0)
    {
        std::cerr << "getaddrinfo() failed: " << gai_strerror(status) << std::endl;
        exit(-1);
    }

    serverFd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
    if (serverFd == -1)
    {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    if (connect(serverFd, servInfo->ai_addr, servInfo->ai_addrlen) == -1)
    {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    int nodelay = 1;
    if (setsockopt(serverFd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&nodelay), sizeof(nodelay)) == -1)
    {
        std::cerr << "setsockopt(TCP_NODELAY) failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    // Josue: This sleep seems required, at least by silo, to allow the server threads to start and be ready on time to receive the client requests
    // Without this sleep, the silo server started to process the requests 1-2 seconds after the clients start sending thus queuing many requests at the begining of the execution
    // Provably the number of seconds could be reduced, but I don't think these 5 seconds negatively affect any experiment
    sleep(5);
}

bool NetworkedClient::send(Request *req)
{

    // std::cerr << "[send] started" << std::endl;

    req->sentNs = getCurNs();

    pthread_mutex_lock(&sendLock);
    int len = 0;
    int sent = 0;

    req->sentNs_lock_ad = getCurNs();

    if (numReqsSent != (warmupreqs + maxreqs))
    {
        len = sizeof(Request) - MAX_REQ_BYTES + req->len;
        // std::cerr << "[send] sendfull started" << std::endl;
        sent = sendfull(serverFd, reinterpret_cast<const char *>(req), len, 0);
        // std::cerr << "[send] sendfull done" << std::endl;
        if (sent != len)
        {
            error = strerror(errno);
        }

        // fprintf(stderr, "Sent req %llu at %.3f\n", (unsigned long long) req->id, (unsigned long long) req->sentNs_lock_ad / (double) 1000000);
        // std::cerr << "[send] p1" << std::endl;
        // req->sentNs_lock_re = getCurNs();   // Josue: commented to fix core dumped with specjbb
        //  std::cerr << "[send] p2" << std::endl;
        // send_req_ns += req->sentNs_lock_re - req->sentNs_lock_ad;  // Josue: commented to fix core dumped with specjbb
        // std::cerr << "[send] p3" << std::endl;
        // total_req_ns += (req->startNs_lock_re - req->startNs) + (req->sentNs_lock_re - req->sentNs);  // Josue: commented to fix core dumped with specjbb
        // std::cerr << "[send] p4" << std::endl;
        numReqsSent++;
        sent_requests++;
    }
    // std::cerr << "[send] p5" << std::endl;
    pthread_mutex_unlock(&sendLock);
    // std::cerr << "[send] going to return" << std::endl;
    return (sent == len);
}

bool NetworkedClient::recv(Response *resp)
{

    pthread_mutex_lock(&recvLock);

    uint64_t s1 = getCurNs();

    int len = sizeof(Response) - MAX_RESP_BYTES; // Read request header first
    int recvd = recvfull(serverFd, reinterpret_cast<char *>(resp), len, 0);
    if (recvd != len)
    {
        error = strerror(errno);
        return false;
    }

    if (resp->type == RESPONSE)
    {
        recvd = recvfull(serverFd, reinterpret_cast<char *>(&resp->data),
                         resp->len, 0);

        if (static_cast<size_t>(recvd) != resp->len)
        {
            error = strerror(errno);
            return false;
        }
    }

    resp->recvNs = getCurNs() - s1;

    pthread_mutex_unlock(&recvLock);

    return true;
}

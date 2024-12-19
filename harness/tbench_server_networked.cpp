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

#include "tbench_server.h"

#include <algorithm>
#include <atomic>
#include <vector>

#include "helpers.h"
#include "server.h"

#include <assert.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include <inttypes.h>

#include <time.h>
#include <fstream>

/*******************************************************************************
 * NetworkedServer
 *******************************************************************************/
NetworkedServer::NetworkedServer(int nthreads, std::string ip, int port,
                                 int nclients, std::string str_file_name)
    : Server(nthreads)
{
    pthread_mutex_init(&sendLock, nullptr);
    pthread_mutex_init(&recvLock, nullptr);

    reqbuf = new Request[nthreads];

    activeFds.resize(nthreads);

    recvClientHead = 0;

    // Get address info
    int status;
    struct addrinfo hints;
    struct addrinfo *servInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream portstr;
    portstr << port;

    start_file_name = str_file_name;

    const char *ipstr = (ip.size() > 0) ? ip.c_str() : nullptr;

    if ((status = getaddrinfo(ipstr, portstr.str().c_str(), &hints, &servInfo)) != 0)
    {
        std::cerr << "getaddrinfo() failed: " << gai_strerror(status)
                  << std::endl;
        exit(-1);
    }

    // Create listening socket
    int listener = socket(servInfo->ai_family, servInfo->ai_socktype,
                          servInfo->ai_protocol);
    if (listener == -1)
    {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        std::cerr << "setsockopt() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    if (bind(listener, servInfo->ai_addr, servInfo->ai_addrlen) == -1)
    {
        std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    if (listen(listener, 10) == -1)
    {
        std::cerr << "listen() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    // SERVER NEW CONNECTIONS //
    listenFd = listener;

    // Establish connections with clients
    // struct sockaddr_storage clientAddr;
    // socklen_t clientAddrSize;

    // for (int c = 0; c < nclients; ++c) {
    //     clientAddrSize = sizeof(clientAddr);
    //     memset(&clientAddr, 0, clientAddrSize);

    //     int clientFd = accept(listener, \
    //             reinterpret_cast<struct sockaddr*>(&clientAddr), \
    //             &clientAddrSize);

    //     if (clientFd == -1) {
    //         std::cerr << "accept() failed: " << strerror(errno) << std::endl;
    //         exit(-1);
    //     }

    //     int nodelay = 1;
    //     if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY,
    //             reinterpret_cast<char*>(&nodelay), sizeof(nodelay)) == -1) {
    //         std::cerr << "setsockopt(TCP_NODELAY) failed: " << strerror(errno) \
    //             << std::endl;
    //         exit(-1);
    //     }

    //     clientFds.push_back(clientFd);
    // }
    // ------------------------ //
}

NetworkedServer::~NetworkedServer()
{
    delete reqbuf;
}

void NetworkedServer::removeClient(int fd)
{
    auto it = std::find(clientFds.begin(), clientFds.end(), fd);
    clientFds.erase(it);
}

bool NetworkedServer::checkRecv(int recvd, int expected, int fd)
{
    bool success = false;
    if (recvd == 0)
    { // Client exited
        std::cerr << "Client left, removing" << std::endl;
        removeClient(fd);
        success = false;
    }
    else if (recvd == -1)
    {
        std::cerr << "recv() failed: " << strerror(errno)
                  << ". Exiting" << std::endl;
        exit(-1);
    }
    else
    {
        if (recvd != expected)
        {
            std::cerr << "ERROR! recvd = " << recvd << ", expected = "
                      << expected << std::endl;
            exit(-1);
        }
        // assert(recvd == expected);
        success = true;
    }

    return success;
}

// size_t NetworkedServer::recvReq(int id, void** data) {
//     pthread_mutex_lock(&recvLock);

//     bool success = false;
//     Request* req;
//     int fd = -1;

//     while (!success && clientFds.size() > 0) {
//         int maxFd = -1;
//         fd_set readSet;
//         FD_ZERO(&readSet);
//         for (int f : clientFds) {
//             FD_SET(f, &readSet);
//             if (f > maxFd) maxFd = f;
//         }

//         int ret = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
//         if (ret == -1) {
//             std::cerr << "select() failed: " << strerror(errno) << std::endl;
//             exit(-1);
//         }

//         fd = -1;

//         for (size_t i = 0; i < clientFds.size(); ++i) {
//             size_t idx = (recvClientHead + i) % clientFds.size();
//             if (FD_ISSET(clientFds[idx], &readSet)) {
//                 fd = clientFds[idx];
//                 break;
//             }
//         }

//         recvClientHead = (recvClientHead + 1) % clientFds.size();

//         assert(fd != -1);

//         int len = sizeof(Request) - MAX_REQ_BYTES; // Read request header first

//         req = &reqbuf[id];
//         int recvd = recvfull(fd, reinterpret_cast<char*>(req), len, 0);

//         success = checkRecv(recvd, len, fd);
//         if (!success) continue;

//         recvd = recvfull(fd, req->data, req->len, 0);

//         success = checkRecv(recvd, req->len, fd);
//         if (!success) continue;
//     }

//     if (clientFds.size() == 0) {
//         std::cerr << "All clients exited. Server finishing" << std::endl;
//         exit(0);
//     } else {
//         uint64_t curNs = getCurNs();
//         reqInfo[id].id = req->id;
//         reqInfo[id].startNs = curNs;
//         activeFds[id] = fd;

//         *data = reinterpret_cast<void*>(&req->data);
//     }

//     if (!start_file) {
//         start_file = true;
//         time_t rawtime;
//         struct tm * timeinfo;
//         time (&rawtime);
//         timeinfo = localtime (&rawtime);
//         fprintf(stderr, "\n[SERVER] Starting to reply clients -- %s", asctime (timeinfo));
//         if (!start_file_name.empty()) {
//             std::ofstream file {start_file_name};
//         }
//     }

//     pthread_mutex_unlock(&recvLock);

//     return req->len;
// };

// SERVER DOESNT DIE //
size_t NetworkedServer::recvReq(int id, void **data)
{
    pthread_mutex_lock(&recvLock);

    bool success = false;
    Request *req;
    int fd = -1;

    while (!success)
    {
        if (clientFds.size() == 0)
        {
            // SERVER NEW CONNECTIONS //
            int maxFd = listenFd;
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(listenFd, &readSet);
            int ret = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
            if (ret == -1)
            {
                std::cerr << "select() failed: " << strerror(errno) << std::endl;
                pthread_mutex_unlock(&recvLock);
                exit(-1);
            }

            if (checkNewClient(&readSet))
            {
                recvClientHead = 0;
            }
            // ----------------------- //

            continue;
        }

        int maxFd = -1;
        fd_set readSet;
        FD_ZERO(&readSet);

        // SERVER NEW CONNECTIONS //
        //add listening socket to the set
        FD_SET(listenFd, &readSet);
        maxFd = listenFd;

        for (int f : clientFds)
        {
            FD_SET(f, &readSet);
            if (f > maxFd)
                maxFd = f;
        }

        int ret = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
        if (ret == -1)
        {
            std::cerr << "select() failed: " << strerror(errno) << std::endl;
            exit(-1);
        }

        //if new client is detected, actualiza fd_Set
        if (checkNewClient(&readSet))
        {
            FD_ZERO(&readSet);
            maxFd = -1;
            for (int f : clientFds)
            {
                FD_SET(f, &readSet);
                if (f > maxFd)
                    maxFd = f;
            }
            ret = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
            if (ret == -1) {
                std::cerr << "select() failed: " << strerror(errno) << std::endl;
                pthread_mutex_unlock(&recvLock);
                exit(-1);
            }
        }
        // ----------------------- //

        fd = -1;

        for (size_t i = 0; i < clientFds.size(); ++i)
        {
            size_t idx = (recvClientHead + i) % clientFds.size();
            if (FD_ISSET(clientFds[idx], &readSet))
            {
                fd = clientFds[idx];
                break;
            }
        }

        recvClientHead = (recvClientHead + 1) % clientFds.size();

        if (fd == -1)
        {
            continue;
        }

        int len = sizeof(Request) - MAX_REQ_BYTES; // Read request header first

        req = &reqbuf[id];
        int recvd = recvfull(fd, reinterpret_cast<char *>(req), len, 0);

        success = checkRecv(recvd, len, fd);
        if (!success)
            continue;

        recvd = recvfull(fd, req->data, req->len, 0);

        success = checkRecv(recvd, req->len, fd);
        if (!success)
            continue;
    }

    uint64_t curNs = getCurNs();
    reqInfo[id].id = req->id;
    reqInfo[id].startNs = curNs;
    activeFds[id] = fd;

    *data = reinterpret_cast<void *>(&req->data);

    if (!start_file)
    {
        start_file = true;
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        fprintf(stderr, "\n[SERVER] Starting to reply clients -- %s", asctime(timeinfo));
        if (!start_file_name.empty())
        {
            std::ofstream file{start_file_name};
        }
    }

    pthread_mutex_unlock(&recvLock);

    return req->len;
}
// ------------------------ //

void NetworkedServer::sendResp(int id, const void *data, size_t len)
{
    pthread_mutex_lock(&sendLock);

    Response *resp = new Response();

    resp->type = RESPONSE;
    resp->id = reqInfo[id].id;
    resp->len = len;
    memcpy(reinterpret_cast<void *>(&resp->data), data, len);

    uint64_t curNs = getCurNs();

    if (curNs <= reqInfo[id].startNs)
    {
        fprintf(stderr, "Next assert is going to fail (2)\n");
        fprintf(stderr, "CurNs: %llu reqInfo[id].startNs: %llu -- id: %d reqInfo[id].id: %llu\n", (unsigned long long)curNs, (unsigned long long)reqInfo[id].startNs, id, (unsigned long long)reqInfo[id].id);
    }
    assert(curNs > reqInfo[id].startNs);
    resp->svcNs = curNs - reqInfo[id].startNs;

    // fprintf(stderr, " SERVER_LOG -- Resp %llu StartNs: %llu SendRespNs: %llu\n -- sncNs: %llu", (unsigned long long) resp->id, (unsigned long long) reqInfo[id].startNs, (unsigned long long) curNs, (unsigned long long) resp->svcNs);

    fprintf(stderr, " SERVER_LOG -- Resp %llu StartNs: %llu SendRespNs: %llu -- sncNs1: %llu sncNs2: %llu\n", (unsigned long long)resp->id, (unsigned long long)reqInfo[id].startNs, (unsigned long long)curNs, (unsigned long long)resp->svcNs, (unsigned long long)(curNs - reqInfo[id].startNs));

    // resp->startNs = reqInfo[id].startNs;
    // resp->endNs = curNs;

    int fd = activeFds[id];
    int totalLen = sizeof(Response) - MAX_RESP_BYTES + len;
    int sent = sendfull(fd, reinterpret_cast<const char *>(resp), totalLen, 0);
    // assert(sent == totalLen);

    // if (finishedReqs>warmupReqs){
    //     fprintf(stderr, "Response send: currNs=%llu id=%llu\n", (unsigned long long) curNs, (unsigned long long) resp->id);
    // }

    ++finishedReqs;

    // if (finishedReqs == warmupReqs) {

    //     fprintf(stderr, "Warmup requests (%llu) completed. Send ROI begin!\n", (unsigned long long) warmupReqs);

    //     resp->type = ROI_BEGIN;
    //     for (int fd : clientFds) {
    //         totalLen = sizeof(Response) - MAX_RESP_BYTES;
    //         sent = sendfull(fd, reinterpret_cast<const char*>(resp), totalLen, 0);
    //         assert(sent == totalLen);
    //     }
    // } else if (finishedReqs == warmupReqs + maxReqs) {

    //     fprintf(stderr, "Overall number of requests (%llu) completed. Send FINISH begin!\n", (unsigned long long) finishedReqs);
    //     resp->type = FINISH;
    //     for (int fd : clientFds) {
    //         totalLen = sizeof(Response) - MAX_RESP_BYTES;
    //         sent = sendfull(fd, reinterpret_cast<const char*>(resp), totalLen, 0);
    //         assert(sent == totalLen);
    //     }
    // }

    // if (resp->id < 10)
    //     printf("NetworkedServer::send_response     %llu     %.3f\n", (unsigned long long) resp->id, (unsigned long long) getCurNs() / (double) 1000000);

    delete resp;

    pthread_mutex_unlock(&sendLock);
}

void NetworkedServer::finish()
{
    pthread_mutex_lock(&sendLock);

    printf("---- tBenchServerFinish called!\n");

    Response *resp = new Response();
    resp->type = FINISH;

    for (int fd : clientFds)
    {
        int len = sizeof(Response) - MAX_RESP_BYTES;
        int sent = sendfull(fd, reinterpret_cast<const char *>(resp), len, 0);
        assert(sent == len);
    }

    delete resp;

    pthread_mutex_unlock(&sendLock);
}

// SERVER NEW CONNECTIONS //
bool NetworkedServer::checkNewClient(fd_set *fdSet)
{
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrSize;
    std::cout << "Checking new connection" << std::endl;
    bool res = false;
    if (FD_ISSET(listenFd, fdSet))
    {
        clientAddrSize = sizeof(clientAddr);
        memset(&clientAddr, 0, clientAddrSize);

        int clientFd = accept(listenFd,
                              reinterpret_cast<struct sockaddr *>(&clientAddr),
                              &clientAddrSize);

        if (clientFd == -1)
        {
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
            return false;
        }

        int nodelay = 1;
        if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY,
                       reinterpret_cast<char *>(&nodelay), sizeof(nodelay)) == -1)
        {
            std::cerr << "setsockopt(TCP_NODELAY) failed: " << strerror(errno)
                      << std::endl;
            close(clientFd);
            return false;
        }

        clientFds.push_back(clientFd);
        res = true;
    }
    return res;
}

/*******************************************************************************
 * Per-thread State
 *******************************************************************************/
__thread int tid;

/*******************************************************************************
 * Global data
 *******************************************************************************/
std::atomic_int curTid;
NetworkedServer *server;

/*******************************************************************************
 * API
 *******************************************************************************/
void tBenchServerInit(int nthreads)
{
    curTid = 0;
    std::string serverurl = getOpt<std::string>("TBENCH_SERVER", "");
    int serverport = getOpt<int>("TBENCH_SERVER_PORT", 8080);
    int nclients = getOpt<int>("TBENCH_NCLIENTS", 1);
    std::string str_file_name = getOpt<std::string>("TBENCH_START_FILE", "");
    server = new NetworkedServer(nthreads, serverurl, serverport, nclients, str_file_name);
}

void tBenchServerThreadStart()
{
    tid = curTid++;
}

void tBenchServerFinish()
{
    server->finish();
}

size_t tBenchRecvReq(void **data)
{
    return server->recvReq(tid, data);
}

void tBenchSendResp(const void *data, size_t size)
{
    return server->sendResp(tid, data, size);
}

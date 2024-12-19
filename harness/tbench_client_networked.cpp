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

#include <assert.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include <time.h>

const time_t ctt = time(0);

void *send(void *c)
{
  // std::cerr << "[SEND] - Init started" << std::endl;
  NetworkedClient *client = reinterpret_cast<NetworkedClient *>(c);
  // std::cerr << "[SEND] - Init completed" << std::endl;
  while (true)
  {
    //If the threads already sent the numbre of queries, then finish
    uint64_t numReqsSent = client->get_reqsSent();
    if (numReqsSent >= (client->get_warmupreq() + client->get_maxreq()))
    {
      // std::cerr << "[SEND] Max requests reached" << std::endl;
      pthread_exit(nullptr);
    }
    // uint64_t A = getCurNs();
    // std::cerr << "[SEND] Going to start request" << std::endl;
    Request *req = client->startReq();
    // std::cerr << "[SEND] Request started" << std::endl;
    //  uint64_t B = getCurNs();
    if (!client->send(req))
    {
      std::cerr << "[CLIENT] send() failed : " << client->errmsg() << std::endl;
      std::cerr << "[CLIENT] Not sending further request" << std::endl;
      break; // We are done
    }
    // std::cerr << "[SEND] Request sent" << std::endl;
    //  uint64_t C = getCurNs();
    //  std::cerr << "JJJJJJJJJJ _startReq_ " << B - A << " _sendNs_ " << C - B << std::endl;
  }
  return nullptr;
}

void *recv(void *c)
{
  // std::cerr << "[RECV] - Init started" << std::endl;
  NetworkedClient *client = reinterpret_cast<NetworkedClient *>(c);
  // std::cerr << "[RECV] - Init completed" << std::endl;

  Response resp;
  while (true)
  {
    // std::cerr << "[RECV] Waiting for a response" << std::endl;
    if (!client->recv(&resp))
    {
      std::cerr << "[CLIENT] recv() failed : " << client->errmsg() << std::endl;
      return nullptr;
    }

    // std::cerr << "[RECV] Response received" << std::endl;

    if (resp.type == RESPONSE)
    {
      // std::cerr << "[RECV] Response - finiReq started" << std::endl;
      client->finiReq(&resp);
      // std::cerr << "[RECV] Response - finiReq completed" << std::endl;
    }
    // else if (resp.type == ROI_BEGIN)
    // {
    //   fprintf(stderr, "--> Client recieved ROI_BEGIN\n");
    //   client->startRoi();
    // }
    // else if (resp.type == FINISH)
    // {
    //   fprintf(stderr, "--> Client received FINISH\n");
    //   client->dumpStats();
    //   syscall(SYS_exit_group, 0);
    // }
    else
    {
      std::cerr << "Unknown response type: " << resp.type << std::endl;
      return nullptr;
    }
  }
}

int main(int argc, char *argv[])
{
  int nthreads = getOpt<int>("TBENCH_CLIENT_THREADS", 1);
  std::string server = getOpt<std::string>("TBENCH_SERVER", "");
  int serverport = getOpt<int>("TBENCH_SERVER_PORT", 8080);
  int client_id = getOpt<int>("TBENCH_ID", 0);

  // CLIENT CONTROLS QUERIES //
  uint64_t warmup = getOpt<uint64_t>("TBENCH_WARMUPREQS", 10);
  uint64_t maxreq = getOpt<uint64_t>("TBENCH_MAXREQS", 100);
  // ----------------------- //
  if(warmup == 0 || maxreq == 0) {
    std::cout << "[CLIENT]: ERROR warmupreqs and maxreqs cannot be 0" << std::endl;
    return 0;
  }
  NetworkedClient *client = new NetworkedClient(nthreads, server, serverport, client_id, warmup, maxreq);

  std::cout << "Client created" << std::endl;

  std::vector<pthread_t> senders(nthreads);
  std::vector<pthread_t> receivers(nthreads);

  for (int t = 0; t < nthreads; ++t)
  {
    int status = pthread_create(&senders[t], nullptr, send,
                                reinterpret_cast<void *>(client));
    assert(status == 0);
  }

  for (int t = 0; t < nthreads; ++t)
  {
    int status = pthread_create(&receivers[t], nullptr, recv,
                                reinterpret_cast<void *>(client));
    assert(status == 0);
  }

  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  fprintf(stderr, "\n[CLIENT] All threads created -- %s", asctime(timeinfo));

  for (int t = 0; t < nthreads; ++t)
  {
    int status;
    status = pthread_join(senders[t], nullptr);
    assert(status == 0);

    status = pthread_join(receivers[t], nullptr);
    assert(status == 0);
  }

  return 0;
}

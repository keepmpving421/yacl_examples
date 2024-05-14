#include <future>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "yacl/base/buffer.h"
#include "yacl/link/context.h"
#include "yacl/link/factory.h"
#include "yacl/link/link.h"
#include "yacl/link/link.pb.h"

using yacl::ByteContainerView;
using yacl::link::Context;
using yacl::link::ContextDesc;
using yacl::link::ContextDescProto;
using yacl::link::FactoryMem;
using yacl::link::RetryOptionsProto;
using yacl::link::FactoryBrpc;

static bool stop_signal = false;

void sig_handler(int sig_num) {
  SPDLOG_INFO("Recv signal {}, stop send or recv loop.", sig_num);
  stop_signal = true;
}

int main(int argc, char **argv) {
  signal(SIGINT, sig_handler);

  spdlog::set_level(spdlog::level::trace);

  std::string uuid = std::string(argv[argc - 1]);
  if (uuid != "party_1" && uuid != "party_2") {
    SPDLOG_ERROR("Party id must be 'party_1' or 'party_2'.");
    return -1;
  }

  std::string party_1_host = "127.0.0.1:16001";
  std::string party_2_host = "127.0.0.1:17001";

  if (uuid == "party_1") {
    SPDLOG_INFO("This party is {}, host {}.", uuid, party_1_host);
  } else {
    SPDLOG_INFO("This party is {}, host {}.", uuid, party_2_host);
  }

  ContextDescProto proto;
  proto.set_id(std::string("root"));
  
  auto party = proto.add_parties();
  party->set_id(std::string("root"));
  party->set_host(party_1_host);

  party = proto.add_parties();
  party->set_id(std::string("root"));
  party->set_host(party_2_host);

  auto retry_opts = proto.mutable_retry_opts();
  // Retry 10 times total;
  retry_opts->set_max_retry(10);
  // wait 10s before first retry;
  retry_opts->set_retry_interval_ms(1000);
  // Add 5s to the wait time before next retry;
  retry_opts->set_retry_interval_incr_ms(5000);
  // Max wait time of next retry;
  retry_opts->set_max_retry_interval_ms(51000);

  SPDLOG_INFO(proto.DebugString());

  ContextDesc ctx_desc(proto);
  
  std::shared_ptr<Context> ctx;
  if (uuid == "party_1")
    ctx = FactoryBrpc().CreateContext(ctx_desc, 0);
  else
    ctx = FactoryBrpc().CreateContext(ctx_desc, 1);

  ctx->ConnectToMesh();

  SPDLOG_INFO("Num party is {}.", ctx->WorldSize());
  SPDLOG_INFO("Connect to peer finish.");

  if (uuid == "party_1") {
    std::vector<char> test_data;
    uint32_t test_size = 1024 * 1024 * 1024;
    test_size = test_size * 500;
    test_data.resize(test_size);

    while (!stop_signal) {
      // Always send.
      ByteContainerView send_value(test_data.data(), test_data.size());
      ctx->Send(ctx->NextRank(), send_value, "500MB_chunk");

      SPDLOG_INFO("Finish send.");
      sleep(5);
    }
  } else {
    while (!stop_signal) {
      // Always recv.
      auto recv_data = ctx->Recv(ctx->PrevRank(), "500MB_chunk");
      SPDLOG_INFO("Finish recv.");
    }
  }

  return 0;
}

//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <stdint.h>
#include <atomic>
#include <memory>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mindalpha/node_info.h>
#include <mindalpha/message.h>
#include <mindalpha/actor_config.h>
#include <mindalpha/message_transport.h>
#include <mindalpha/node_manager.h>

namespace mindalpha
{

class ActorProcess
{
    friend class PSAgent;

public:
    explicit ActorProcess(std::shared_ptr<ActorConfig> config);

    std::shared_ptr<ActorConfig> GetConfig() const { return config_; }
    void SetConfig(std::shared_ptr<ActorConfig> value) { config_ = std::move(value); }

    int64_t GetMessageId() { return message_counter_++; }
    void Barrier(int group);

    int64_t Send(const Message& msg);
    void Receiving();

    void Start();
    void Run();
    void Stop();

private:
    bool IsReady();
    void SetIsReady(bool value);
    void WaitReady();

    bool HandleDataMessage(Message&& msg);

#undef MINDALPHA_NODE_CONTROL_COMMAND_DEF
#define MINDALPHA_NODE_CONTROL_COMMAND_DEF(n) bool Handle##n##Message(const Message& msg);
    MINDALPHA_NODE_CONTROL_COMMANDS(MINDALPHA_NODE_CONTROL_COMMAND_DEF)

    std::unordered_set<int> GetDeadNodes();
    void UpdateLocalId(const Message& msg);
    void CoordinatorHandleAddNode(const Message& msg);

    std::shared_ptr<ActorConfig> config_;
    std::unique_ptr<MessageTransport> transport_;
    std::unique_ptr<NodeManager> manager_;
    std::unordered_map<std::string, int> connected_nodes_;
    std::unordered_map<int, int> shared_node_mapping_;
    std::vector<int> barrier_counter_;
    bool ready_{false};
    std::mutex ready_mutex_;
    std::condition_variable ready_cv_;
    std::atomic<int64_t> message_counter_{0};
    std::atomic<int64_t> send_bytes_{0};
    std::atomic<int64_t> receive_bytes_{0};
    MessageMeta nodes_;
    MessageMeta recovery_nodes_;
    int num_servers_ = 0;
    int num_workers_ = 0;
    std::mutex start_mutex_;
    std::unique_ptr<std::future<void>> receiver_exit_;
    int init_stage_ = 0;
    NodeInfo coordinator_;
    std::shared_ptr<class PSAgent> agent_;
};

}

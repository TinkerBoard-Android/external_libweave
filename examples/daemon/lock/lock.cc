// Copyright 2015 The Weave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/daemon/common/daemon.h"

#include <weave/device.h>
#include <weave/enum_to_string.h>

#include <base/bind.h>
#include <base/memory/weak_ptr.h>

namespace weave {
namespace lockstate {
enum class LockState { kUnlocked, kLocked, kPartiallyLocked };

const weave::EnumToStringMap<LockState>::Map kLockMapMethod[] = {
    {LockState::kLocked, "locked"},
    {LockState::kUnlocked, "unlocked"},
    {LockState::kPartiallyLocked, "partiallyLocked"}};
}  // namespace lockstate

template <>
EnumToStringMap<lockstate::LockState>::EnumToStringMap()
    : EnumToStringMap(lockstate::kLockMapMethod) {}
}  // namespace weave

// LockHandler is a command handler example that shows
// how to handle commands for a Weave lock.
class LockHandler {
 public:
  LockHandler() = default;
  void Register(weave::Device* device) {
    device_ = device;

    device->AddStateDefinitionsFromJson(R"({
      "lock": {
        "lockedState": {
          "type": "string",
          "enum": ["locked", "unlocked", "partiallyLocked"]
        },
        "isLockingSupported": {"type": "boolean"}
      }
    })");

    device->SetStatePropertiesFromJson(R"({
      "lock":{
        "lockedState": "locked",
        "isLockingSupported": true
      }
    })",
                                       nullptr);

    device->AddCommandDefinitionsFromJson(R"({
      "lock": {
        "setConfig":{
          "minimalRole": "user",
          "parameters": {
            "lockedState": {"type": "string", "enum":["locked", "unlocked"]}
          }
        }
      }
    })");
    device->AddCommandHandler("lock.setConfig",
                              base::Bind(&LockHandler::OnLockSetConfig,
                                         weak_ptr_factory_.GetWeakPtr()));
  }

 private:
  void OnLockSetConfig(const std::weak_ptr<weave::Command>& command) {
    auto cmd = command.lock();
    if (!cmd)
      return;
    LOG(INFO) << "received command: " << cmd->GetName();
    const auto& params = cmd->GetParameters();
    std::string requested_state;
    if (params.GetString("lockedState", &requested_state)) {
      LOG(INFO) << cmd->GetName() << " state: " << requested_state;

      weave::lockstate::LockState new_lock_status;

      if (!weave::StringToEnum(requested_state, &new_lock_status)) {
        // Invalid lock state was specified.
        weave::ErrorPtr error;
        weave::Error::AddTo(&error, FROM_HERE, "example",
                            "invalid_parameter_value", "Invalid parameters");
        cmd->Abort(error.get(), nullptr);
        return;
      }

      if (new_lock_status != lock_state_) {
        lock_state_ = new_lock_status;

        LOG(INFO) << "Lock is now: " << requested_state;
        UpdateLockState();
      }
      cmd->Complete({}, nullptr);
      return;
    }
    weave::ErrorPtr error;
    weave::Error::AddTo(&error, FROM_HERE, "example", "invalid_parameter_value",
                        "Invalid parameters");
    cmd->Abort(error.get(), nullptr);
  }

  void UpdateLockState() {
    base::DictionaryValue state;
    std::string updated_state = weave::EnumToString(lock_state_);
    state.SetString("lock.lockedState", updated_state);
    device_->SetStateProperties(state, nullptr);
  }

  weave::Device* device_{nullptr};

  // Simulate the state of the light.
  weave::lockstate::LockState lock_state_{weave::lockstate::LockState::kLocked};
  base::WeakPtrFactory<LockHandler> weak_ptr_factory_{this};
};

int main(int argc, char** argv) {
  Daemon::Options opts;
  if (!opts.Parse(argc, argv)) {
    Daemon::Options::ShowUsage(argv[0]);
    return 1;
  }
  Daemon daemon{opts};
  LockHandler handler;
  handler.Register(daemon.GetDevice());
  daemon.Run();
  return 0;
}

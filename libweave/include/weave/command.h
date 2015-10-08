// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIBWEAVE_INCLUDE_WEAVE_COMMAND_H_
#define LIBWEAVE_INCLUDE_WEAVE_COMMAND_H_

#include <string>

#include <base/values.h>
#include <weave/error.h>

namespace weave {

enum class CommandStatus {
  kQueued,
  kInProgress,
  kPaused,
  kError,
  kDone,
  kCancelled,
  kAborted,
  kExpired,
};

enum class CommandOrigin { kLocal, kCloud };

class Command {
 public:
  // Returns the full command ID.
  virtual const std::string& GetID() const = 0;

  // Returns the full name of the command.
  virtual const std::string& GetName() const = 0;

  // Returns the command status.
  virtual CommandStatus GetStatus() const = 0;

  // Returns the origin of the command.
  virtual CommandOrigin GetOrigin() const = 0;

  // Returns the command parameters.
  virtual std::unique_ptr<base::DictionaryValue> GetParameters() const = 0;

  // Returns the command progress.
  virtual std::unique_ptr<base::DictionaryValue> GetProgress() const = 0;

  // Returns the command results.
  virtual std::unique_ptr<base::DictionaryValue> GetResults() const = 0;

  // Returns the command error.
  virtual const Error* GetError() const = 0;

  // Updates the command progress. The |progress| should match the schema.
  // Returns false if |progress| value is incorrect.
  virtual bool SetProgress(const base::DictionaryValue& progress,
                           ErrorPtr* error) = 0;

  // Updates the command results. The |results| should match the schema.
  // Returns false if |results| value is incorrect.
  // Sets command into terminal "done" state.
  // TODO(vitalybuka): Rename to Complete.
  virtual bool SetResults(const base::DictionaryValue& results,
                          ErrorPtr* error) = 0;

  // Sets command into paused state.
  // This is not terminal state. Command can be resumed with |SetProgress| call.
  virtual bool Pause(ErrorPtr* error) = 0;

  // Sets command into error state and assign error.
  // This is not terminal state. Command can be resumed with |SetProgress| call.
  virtual bool SetError(const Error* command_error, ErrorPtr* error) = 0;

  // Aborts command execution.
  // Sets command into terminal "aborted" state.
  virtual bool Abort(const Error* command_error, ErrorPtr* error) = 0;

  // Cancels command execution.
  // Sets command into terminal "canceled" state.
  virtual bool Cancel(ErrorPtr* error) = 0;

 protected:
  virtual ~Command() = default;
};

}  // namespace weave

#endif  // LIBWEAVE_INCLUDE_WEAVE_COMMAND_H_

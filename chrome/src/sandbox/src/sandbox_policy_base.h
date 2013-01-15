// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SANDBOX_SRC_SANDBOX_POLICY_BASE_H_
#define SANDBOX_SRC_SANDBOX_POLICY_BASE_H_

#include <windows.h>

#include <list>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/string16.h"
#include "sandbox/src/crosscall_server.h"
#include "sandbox/src/handle_closer.h"
#include "sandbox/src/ipc_tags.h"
#include "sandbox/src/policy_engine_opcodes.h"
#include "sandbox/src/policy_engine_params.h"
#include "sandbox/src/sandbox_policy.h"
#include "sandbox/src/win_utils.h"

namespace sandbox {

class LowLevelPolicy;
class TargetProcess;
struct PolicyGlobal;

// We act as a policy dispatcher, implementing the handler for the "ping" IPC,
// so we have to provide the appropriate handler on the OnMessageReady method.
// There is a static_cast for the handler, and the compiler only performs the
// cast if the first base class is Dispatcher.
class PolicyBase : public Dispatcher, public TargetPolicy {
 public:
  PolicyBase();

  // TargetPolicy:
  virtual void AddRef() OVERRIDE;
  virtual void Release() OVERRIDE;
  virtual ResultCode SetTokenLevel(TokenLevel initial,
                                   TokenLevel lockdown) OVERRIDE;
  virtual ResultCode SetJobLevel(JobLevel job_level,
                                 uint32 ui_exceptions) OVERRIDE;
  virtual ResultCode SetAlternateDesktop(bool alternate_winstation) OVERRIDE;
  virtual std::wstring GetAlternateDesktop() const OVERRIDE;
  virtual ResultCode CreateAlternateDesktop(bool alternate_winstation) OVERRIDE;
  virtual void DestroyAlternateDesktop() OVERRIDE;
  virtual ResultCode SetIntegrityLevel(IntegrityLevel integrity_level) OVERRIDE;
  virtual ResultCode SetDelayedIntegrityLevel(
      IntegrityLevel integrity_level) OVERRIDE;
  virtual void SetStrictInterceptions() OVERRIDE;
  virtual ResultCode AddRule(SubSystem subsystem, Semantics semantics,
                             const wchar_t* pattern) OVERRIDE;
  virtual ResultCode AddDllToUnload(const wchar_t* dll_name);
  virtual ResultCode AddKernelObjectToClose(const char16* handle_type,
                                            const char16* handle_name) OVERRIDE;

  // Dispatcher:
  virtual Dispatcher* OnMessageReady(IPCParams* ipc,
                                     CallbackGeneric* callback) OVERRIDE;
  virtual bool SetupService(InterceptionManager* manager, int service) OVERRIDE;

  // Creates a Job object with the level specified in a previous call to
  // SetJobLevel(). Returns the standard windows of ::GetLastError().
  DWORD MakeJobObject(HANDLE* job);

  // Creates the two tokens with the levels specified in a previous call to
  // SetTokenLevel(). Returns the standard windows of ::GetLastError().
  DWORD MakeTokens(HANDLE* initial, HANDLE* lockdown);

  // Adds a target process to the internal list of targets. Internally a
  // call to TargetProcess::Init() is issued.
  bool AddTarget(TargetProcess* target);

  // Called when there are no more active processes in a Job.
  // Removes a Job object associated with this policy and the target associated
  // with the job.
  bool OnJobEmpty(HANDLE job);

  EvalResult EvalPolicy(int service, CountedParameterSetBase* params);

 private:
  ~PolicyBase();

  // Test IPC providers.
  bool Ping(IPCInfo* ipc, void* cookie);

  // Returns a dispatcher from ipc_targets_.
  Dispatcher* GetDispatcher(int ipc_tag);

  // Sets up interceptions for a new target.
  bool SetupAllInterceptions(TargetProcess* target);

  // Sets up the handle closer for a new target.
  bool SetupHandleCloser(TargetProcess* target);

  // This lock synchronizes operations on the targets_ collection.
  CRITICAL_SECTION lock_;
  // Maintains the list of target process associated with this policy.
  // The policy takes ownership of them.
  typedef std::list<TargetProcess*> TargetSet;
  TargetSet targets_;
  // Standard object-lifetime reference counter.
  volatile LONG ref_count;
  // The user-defined global policy settings.
  TokenLevel lockdown_level_;
  TokenLevel initial_level_;
  JobLevel job_level_;
  uint32 ui_exceptions_;
  bool use_alternate_desktop_;
  bool use_alternate_winstation_;
  // Helps the file system policy initialization.
  bool file_system_init_;
  bool relaxed_interceptions_;
  IntegrityLevel integrity_level_;
  IntegrityLevel delayed_integrity_level_;
  // The array of objects that will answer IPC calls.
  Dispatcher* ipc_targets_[IPC_LAST_TAG];
  // Object in charge of generating the low level policy.
  LowLevelPolicy* policy_maker_;
  // Memory structure that stores the low level policy.
  PolicyGlobal* policy_;
  // The list of dlls to unload in the target process.
  std::vector<std::wstring> blacklisted_dlls_;
  // This is a map of handle-types to names that we need to close in the
  // target process. A null set means we need to close all handles of the
  // given type.
  HandleCloser handle_closer_;

  static HDESK alternate_desktop_handle_;
  static HWINSTA alternate_winstation_handle_;

  DISALLOW_COPY_AND_ASSIGN(PolicyBase);
};

}  // namespace sandbox

#endif  // SANDBOX_SRC_SANDBOX_POLICY_BASE_H_
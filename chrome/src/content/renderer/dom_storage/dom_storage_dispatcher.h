// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DOM_STORAGE_DOM_STORAGE_DISPATCHER_H_
#define CONTENT_RENDERER_DOM_STORAGE_DOM_STORAGE_DISPATCHER_H_
#pragma once

#include "base/memory/ref_counted.h"

class GURL;
struct DOMStorageMsg_Event_Params;
namespace dom_storage {
class DomStorageCachedArea;
}
namespace IPC {
class Message;
}

// Dispatches DomStorage related messages sent to a renderer process from the
// main browser process. There is one instance per child process. Messages
// are dispatched on the main renderer thread. The RenderThreadImpl
// creates an instance and delegates calls to it. This classes also manages
// the collection of DomStorageCachedAreas that are active in the process.
class DomStorageDispatcher {
 public:
  DomStorageDispatcher();
  ~DomStorageDispatcher();

  // Each call to open should be balanced with a call to close.
  scoped_refptr<dom_storage::DomStorageCachedArea> OpenCachedArea(
      int connection_id, int64 namespace_id, const GURL& origin);
  void CloseCachedArea(
      int connection_id, dom_storage::DomStorageCachedArea* area);

  bool OnMessageReceived(const IPC::Message& msg);

 private:
  class ProxyImpl;

  // IPC message handlers
  void OnStorageEvent(const DOMStorageMsg_Event_Params& params);
  void OnAsyncOperationComplete(bool success);

  scoped_refptr<ProxyImpl> proxy_;
};

#endif  // CONTENT_RENDERER_DOM_STORAGE_DOM_STORAGE_DISPATCHER_H_
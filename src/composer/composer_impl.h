// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <unordered_map>

#include "apps/mozart/services/composition2/composer.fidl.h"
#include "apps/mozart/services/composition2/session.fidl.h"
#include "apps/mozart/src/composer/session/session.h"
#include "apps/mozart/src/composer/session/session_handler.h"
#include "lib/mtl/tasks/message_loop.h"
#include "lib/mtl/threading/thread.h"

namespace mozart {
namespace composer {

class ComposerImpl : public mozart2::Composer, public SessionContext {
 public:
  ComposerImpl();
  ~ComposerImpl() override;

  // mozart2::Composer interface methods.
  void CreateSession(
      ::fidl::InterfaceRequest<mozart2::Session> request) override;

  // SessionContext interface methods.
  LinkPtr CreateLink(Session* session, const mozart2::LinkPtr& args) override;

  size_t GetSessionCount() { return session_count_; }

 private:
  friend class SessionHandler;
  void ApplySessionUpdate(std::unique_ptr<SessionUpdate> update);

  void TearDownSession(SessionId id);

  std::unordered_map<SessionId, std::unique_ptr<SessionHandler>> sessions_;
  std::atomic<size_t> session_count_;

  SessionId next_session_id_ = 1;
};

}  // namespace composer
}  // namespace mozart

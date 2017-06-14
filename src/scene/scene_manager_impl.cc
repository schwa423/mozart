// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/scene_manager_impl.h"

#include "apps/mozart/src/scene/renderer/renderer.h"
#include "lib/ftl/functional/make_copyable.h"

namespace mozart {
namespace scene {

SceneManagerImpl::SceneManagerImpl(
    vk::Device vk_device,
    escher::ResourceLifePreserver* life_preserver,
    escher::GpuAllocator* allocator,
    escher::impl::GpuUploader* uploader)
    : session_count_(0),
      vk_device_(vk_device),
      life_preserver_(life_preserver),
      image_factory_(
          std::make_unique<escher::SimpleImageFactory>(life_preserver,
                                                       allocator)),
      gpu_uploader_(uploader),
      renderer_(std::make_unique<Renderer>()) {}

SceneManagerImpl::SceneManagerImpl()
    : SceneManagerImpl(nullptr, nullptr, nullptr, nullptr) {}

SceneManagerImpl::~SceneManagerImpl() {}

void SceneManagerImpl::CreateSession(
    ::fidl::InterfaceRequest<mozart2::Session> request,
    ::fidl::InterfaceHandle<mozart2::SessionListener> listener) {
  SessionId session_id = next_session_id_++;

  auto handler =
      CreateSessionHandler(session_id, std::move(request), std::move(listener));
  sessions_.insert({session_id, std::move(handler)});
  ++session_count_;
}

std::unique_ptr<SessionHandler> SceneManagerImpl::CreateSessionHandler(
    SessionId session_id,
    ::fidl::InterfaceRequest<mozart2::Session> request,
    ::fidl::InterfaceHandle<mozart2::SessionListener> listener) {
  return std::make_unique<SessionHandler>(this, session_id, std::move(request),
                                          std::move(listener));
}

SessionHandler* SceneManagerImpl::FindSession(SessionId id) {
  auto it = sessions_.find(id);
  if (it != sessions_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void SceneManagerImpl::ApplySessionUpdate(
    std::unique_ptr<SessionUpdate> update) {
  auto& session = update->session;
  if (session->is_valid()) {
    for (auto& op : update->ops) {
      if (!session->ApplyOp(op)) {
        FTL_LOG(WARNING) << "mozart::Compositor::SceneManagerImpl::"
                            "ApplySessionUpdate() initiating teardown";
        TearDownSession(session->id());
        return;
      }
    }
  }
}

void SceneManagerImpl::TearDownSession(SessionId id) {
  auto it = sessions_.find(id);
  FTL_DCHECK(it != sessions_.end());
  if (it != sessions_.end()) {
    std::unique_ptr<SessionHandler> handler = std::move(it->second);
    sessions_.erase(it);
    --session_count_;
    handler->TearDown();

    // Don't destroy handler immediately, since it may be the one calling
    // TearDownSession().
    mtl::MessageLoop::GetCurrent()->task_runner()->PostTask(
        ftl::MakeCopyable([handler = std::move(handler)]{}));
  }
}

bool SceneManagerImpl::ExportResource(ResourcePtr resource,
                                      const mozart2::ExportResourceOpPtr& op) {
  FTL_LOG(FATAL) << "SceneManagerImpl::ExportResource() unimplemented";
  return false;
}

ResourcePtr SceneManagerImpl::ImportResource(
    Session* session,
    const mozart2::ImportResourceOpPtr& op) {
  FTL_LOG(FATAL) << "SceneManagerImpl::ImportResource() unimplemented";
  return ResourcePtr();
}

LinkPtr SceneManagerImpl::CreateLink(Session* session,
                                     ResourceId node_id,
                                     const mozart2::LinkPtr& args) {
  // TODO: Create a LinkHolder class that takes args.token and destroys
  // links if that gets signalled

  FTL_DCHECK(args);

  // For now, just create a dumb list of sessions.
  auto link = ftl::MakeRefCounted<Link>(session, node_id);

  FTL_CHECK(link);
  links_.push_back(link);

  return link;
}

void SceneManagerImpl::OnSessionTearDown(Session* session) {
  auto predicate = [session](const LinkPtr& l) {
    return l->session() == session;
  };
  // Remove all links to session.
  links_.erase(std::remove_if(links_.begin(), links_.end(), predicate),
               links_.end());
}

}  // namespace scene
}  // namespace mozart
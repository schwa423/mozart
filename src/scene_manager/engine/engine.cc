// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene_manager/engine/engine.h"

#include <set>

#include "apps/mozart/src/scene_manager/engine/frame_scheduler.h"
#include "apps/mozart/src/scene_manager/engine/frame_timings.h"
#include "apps/mozart/src/scene_manager/engine/session.h"
#include "apps/mozart/src/scene_manager/engine/session_handler.h"
#include "apps/mozart/src/scene_manager/resources/compositor/compositor.h"
#include "apps/mozart/src/scene_manager/resources/nodes/traversal.h"
#include "apps/tracing/lib/trace/event.h"
#include "escher/renderer/paper_renderer.h"
#include "lib/ftl/functional/make_copyable.h"

namespace scene_manager {

Engine::Engine(DisplayManager* display_manager,
               escher::Escher* escher,
               std::unique_ptr<escher::VulkanSwapchain> swapchain)
    : display_manager_(display_manager),
      escher_(escher),
      paper_renderer_(ftl::MakeRefCounted<escher::PaperRenderer>(escher)),
      image_factory_(std::make_unique<escher::SimpleImageFactory>(
          escher->resource_recycler(),
          escher->gpu_allocator())),
      rounded_rect_factory_(
          std::make_unique<escher::RoundedRectFactory>(escher)),
      release_fence_signaller_(std::make_unique<ReleaseFenceSignaller>(
          escher->command_buffer_sequencer())),
      swapchain_(std::move(swapchain)),
      session_count_(0) {
  FTL_DCHECK(display_manager_);
  FTL_DCHECK(escher_);
  FTL_DCHECK(swapchain_);

  InitializeFrameScheduler();
  paper_renderer_->set_sort_by_pipeline(false);
}

Engine::Engine(DisplayManager* display_manager,
               std::unique_ptr<ReleaseFenceSignaller> release_fence_signaller)
    : display_manager_(display_manager),
      escher_(nullptr),
      release_fence_signaller_(std::move(release_fence_signaller)) {
  FTL_DCHECK(display_manager_);

  InitializeFrameScheduler();
}

Engine::~Engine() = default;

void Engine::InitializeFrameScheduler() {
  if (display_manager_->default_display()) {
    frame_scheduler_ =
        std::make_unique<FrameScheduler>(display_manager_->default_display());
    frame_scheduler_->set_delegate(this);
  }
}

ResourceLinker& Engine::GetResourceLinker() {
  return resource_linker_;
}

bool Engine::ExportResource(ResourcePtr resource, mx::eventpair endpoint) {
  return resource_linker_.ExportResource(std::move(resource),
                                         std::move(endpoint));
}

void Engine::ImportResource(ImportPtr import,
                            mozart2::ImportSpec spec,
                            const mx::eventpair& endpoint) {
  // The import is not captured in the OnImportResolvedCallback because we don't
  // want the reference in the bind to prevent the import from being collected.
  // However, when the import dies, its handle is collected which will cause the
  // resource to expire within the resource linker. In that case, we will never
  // receive the callback with |ResolutionResult::kSuccess|.
  ResourceLinker::OnImportResolvedCallback import_resolved_callback =
      std::bind(&Engine::OnImportResolvedForResource,  // method
                this,                                  // target
                import.get(),  // the import that will be resolved by the linker
                std::placeholders::_1,  // the acutal object to link to import
                std::placeholders::_2   // result of the linking
      );
  resource_linker_.ImportResource(spec, endpoint, import_resolved_callback);
}

void Engine::OnImportResolvedForResource(
    Import* import,
    ResourcePtr actual,
    ResourceLinker::ResolutionResult resolution_result) {
  if (resolution_result == ResourceLinker::ResolutionResult::kSuccess) {
    actual->AddImport(import);
  }
}

void Engine::ScheduleSessionUpdate(uint64_t presentation_time,
                                   ftl::RefPtr<Session> session) {
  if (session->is_valid()) {
    updatable_sessions_.push({presentation_time, std::move(session)});
    ScheduleUpdate(presentation_time);
  }
}

void Engine::ScheduleUpdate(uint64_t presentation_time) {
  if (frame_scheduler_) {
    frame_scheduler_->RequestFrame(presentation_time);
  } else {
    // Apply update immediately.  This is done for tests.
    FTL_LOG(WARNING)
        << "No FrameScheduler available; applying update immediately";
    RenderFrame(FrameTimingsPtr(), presentation_time, 0);
  }
}

void Engine::CreateSession(
    ::fidl::InterfaceRequest<mozart2::Session> request,
    ::fidl::InterfaceHandle<mozart2::SessionListener> listener) {
  SessionId session_id = next_session_id_++;

  auto handler =
      CreateSessionHandler(session_id, std::move(request), std::move(listener));
  sessions_.insert({session_id, std::move(handler)});
  ++session_count_;
}

std::unique_ptr<DisplaySwapchain> Engine::CreateDisplaySwapchain(
    Display* display) {
  FTL_DCHECK(!display->is_claimed());
  return std::make_unique<DisplaySwapchain>(display, event_timestamper(),
                                            escher(), GetVulkanSwapchain());
}

std::unique_ptr<SessionHandler> Engine::CreateSessionHandler(
    SessionId session_id,
    ::fidl::InterfaceRequest<mozart2::Session> request,
    ::fidl::InterfaceHandle<mozart2::SessionListener> listener) {
  return std::make_unique<SessionHandler>(this, session_id, std::move(request),
                                          std::move(listener));
}

SessionHandler* Engine::FindSession(SessionId id) {
  auto it = sessions_.find(id);
  if (it != sessions_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void Engine::TearDownSession(SessionId id) {
  auto it = sessions_.find(id);
  FTL_DCHECK(it != sessions_.end());
  if (it != sessions_.end()) {
    std::unique_ptr<SessionHandler> handler = std::move(it->second);
    sessions_.erase(it);
    FTL_DCHECK(session_count_ > 0);
    --session_count_;
    handler->TearDown();

    // Don't destroy handler immediately, since it may be the one calling
    // TearDownSession().
    mtl::MessageLoop::GetCurrent()->task_runner()->PostTask(
        ftl::MakeCopyable([handler = std::move(handler)]{}));
  }
}

void Engine::RenderFrame(const FrameTimingsPtr& timings,
                         uint64_t presentation_time,
                         uint64_t presentation_interval) {
  TRACE_DURATION("gfx", "RenderFrame", "frame_number", timings->frame_number(),
                 "time", presentation_time, "interval", presentation_interval);

  if (!ApplyScheduledSessionUpdates(presentation_time, presentation_interval))
    return;

  UpdateAndDeliverMetrics(presentation_time);

  for (auto& compositor : compositors_) {
    compositor->DrawFrame(timings, paper_renderer_.get());
  }
}

bool Engine::ApplyScheduledSessionUpdates(uint64_t presentation_time,
                                          uint64_t presentation_interval) {
  TRACE_DURATION("gfx", "ApplyScheduledSessionUpdates", "time",
                 presentation_time, "interval", presentation_interval);

  bool needs_render = false;
  while (!updatable_sessions_.empty() &&
         updatable_sessions_.top().first <= presentation_time) {
    auto session = std::move(updatable_sessions_.top().second);
    updatable_sessions_.pop();
    if (session) {
      needs_render |= session->ApplyScheduledUpdates(presentation_time,
                                                     presentation_interval);
    } else {
      // Corresponds to a call to ScheduleUpdate(), which always triggers a
      // render.
      needs_render = true;
    }
  }
  return needs_render;
}

escher::VulkanSwapchain Engine::GetVulkanSwapchain() const {
  FTL_DCHECK(swapchain_);
  return *(swapchain_.get());
}

void Engine::AddCompositor(Compositor* compositor) {
  FTL_DCHECK(compositor);
  FTL_DCHECK(compositor->session()->engine() == this);

  bool success = compositors_.insert(compositor).second;
  FTL_DCHECK(success);
}

void Engine::RemoveCompositor(Compositor* compositor) {
  FTL_DCHECK(compositor);
  FTL_DCHECK(compositor->session()->engine() == this);

  size_t count = compositors_.erase(compositor);
  FTL_DCHECK(count == 1);
}

void Engine::UpdateAndDeliverMetrics(uint64_t presentation_time) {
  TRACE_DURATION("gfx", "UpdateAndDeliverMetrics", "time", presentation_time);

  // Gather all of the scene which might need to be updated.
  std::set<Scene*> scenes;
  for (auto compositor : compositors_) {
    compositor->CollectScenes(&scenes);
  }
  if (scenes.empty())
    return;

  // TODO(MZ-216): Traversing the whole graph just to compute this is pretty
  // inefficient.  We should optimize this.
  mozart2::Metrics metrics;
  metrics.scale_x = 1.f;
  metrics.scale_y = 1.f;
  metrics.scale_z = 1.f;
  std::vector<Node*> updated_nodes;
  for (auto scene : scenes) {
    UpdateMetrics(scene, metrics, &updated_nodes);
  }

  // TODO(MZ-216): Deliver events to sessions in batches.
  // We probably want delivery to happen somewhere else which can also
  // handle delivery of other kinds of events.  We should probably also
  // have some kind of backpointer from a session to its handler.
  for (auto node : updated_nodes) {
    if (node->session()) {
      auto event = mozart2::Event::New();
      event->set_metrics(mozart2::MetricsEvent::New());
      event->get_metrics()->node_id = node->id();
      event->get_metrics()->metrics = node->reported_metrics().Clone();

      // We shouldn't be flushing every time here.
      SessionHandler* handler = FindSession(node->session()->id());
      FTL_DCHECK(handler);
      handler->EnqueueEvent(std::move(event));
      handler->FlushEvents(presentation_time);
    }
  }
}

void Engine::UpdateMetrics(Node* node,
                           const mozart2::Metrics& parent_metrics,
                           std::vector<Node*>* updated_nodes) {
  mozart2::Metrics local_metrics;
  local_metrics.scale_x = parent_metrics.scale_x * node->scale().x;
  local_metrics.scale_y = parent_metrics.scale_y * node->scale().y;
  local_metrics.scale_z = parent_metrics.scale_z * node->scale().z;

  if ((node->event_mask() & mozart2::kMetricsEventMask) &&
      !node->reported_metrics().Equals(local_metrics)) {
    node->set_reported_metrics(local_metrics);
    updated_nodes->push_back(node);
  }

  ForEachDirectDescendantFrontToBack(
      *node, [this, &local_metrics, updated_nodes](Node* node) {
        UpdateMetrics(node, local_metrics, updated_nodes);
      });
}

}  // namespace scene_manager

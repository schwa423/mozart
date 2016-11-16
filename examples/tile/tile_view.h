// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_EXAMPLES_TILE_TILE_VIEW_H_
#define APPS_MOZART_EXAMPLES_TILE_TILE_VIEW_H_

#include <map>
#include <memory>

#include "apps/modular/lib/app/application_context.h"
#include "apps/modular/services/application/application_launcher.fidl.h"
#include "apps/mozart/examples/tile/tile_params.h"
#include "apps/mozart/lib/view_framework/base_view.h"
#include "apps/mozart/services/presentation/presenter.fidl.h"
#include "apps/mozart/services/views/view_provider.fidl.h"
#include "lib/fidl/cpp/bindings/binding_set.h"
#include "lib/ftl/macros.h"

namespace examples {

class TileView : public mozart::BaseView,
                 public modular::ApplicationEnvironmentHost,
                 public mozart::Presenter {
 public:
  TileView(mozart::ViewManagerPtr view_manager,
           fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request,
           modular::ApplicationContext* application_context,
           const TileParams& tile_params);

  ~TileView() override;

 private:
  struct ViewData {
    explicit ViewData(const std::string& url,
                      uint32_t key,
                      modular::ApplicationControllerPtr controller);
    ~ViewData();

    const std::string url;
    const uint32_t key;
    modular::ApplicationControllerPtr controller;

    mozart::RectF layout_bounds;
    mozart::ViewPropertiesPtr view_properties;
    mozart::ViewInfoPtr view_info;
    uint32_t scene_version = 1u;
  };

  // |BaseView|:
  void OnLayout() override;
  void OnDraw() override;
  void OnChildAttached(uint32_t child_key,
                       mozart::ViewInfoPtr child_view_info) override;
  void OnChildUnavailable(uint32_t child_key) override;

  void ConnectViews();
  void UpdateScene();

  // |Presenter|:
  void Present(::fidl::InterfaceHandle<mozart::ViewOwner> view_owner) override;

  void PresentHelper(::fidl::InterfaceHandle<mozart::ViewOwner> view_owner,
                     const std::string& url,
                     modular::ApplicationControllerPtr);

  // |ApplicationEnvironmentHost|:
  void GetApplicationEnvironmentServices(
      fidl::InterfaceRequest<modular::ServiceProvider> environment_services)
      override;

  void CreateNestedEnvironment();

  // Nested environment within which the apps started by TileView will run.
  modular::ApplicationEnvironmentPtr env_;
  modular::ApplicationEnvironmentControllerPtr env_controller_;
  fidl::Binding<modular::ApplicationEnvironmentHost> env_host_binding_;
  modular::ServiceProviderImpl env_services_;
  modular::ApplicationLauncherPtr env_launcher_;

  // Context inherited when TileView is launched.
  modular::ApplicationContext* application_context_;

  TileParams params_;

  // The key assigned to the last |View| we added. Incremented every time we
  // present a new |View|.
  uint32_t child_key_ = 0;

  // Map from keys to |ViewData|
  std::map<uint32_t, std::unique_ptr<ViewData>> views_;

  fidl::BindingSet<mozart::Presenter> presenter_bindings_;

  FTL_DISALLOW_COPY_AND_ASSIGN(TileView);
};

}  // namespace examples

#endif  // APPS_MOZART_EXAMPLES_TILE_TILE_VIEW_H_

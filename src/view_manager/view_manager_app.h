// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_VIEW_MANAGER_VIEW_MANAGER_APP_H_
#define APPS_MOZART_SRC_VIEW_MANAGER_VIEW_MANAGER_APP_H_

#include <memory>

#include "application/lib/app/application_context.h"
#include "apps/mozart/services/views/view_manager.fidl.h"
#include "apps/mozart/src/view_manager/view_registry.h"
#include "lib/fidl/cpp/bindings/binding_set.h"
#include "lib/ftl/macros.h"

namespace view_manager {

class ViewManagerImpl;

// View manager application entry point.
class ViewManagerApp {
 public:
  ViewManagerApp();
  ~ViewManagerApp();

 private:
  std::unique_ptr<modular::ApplicationContext> application_context_;
  std::unique_ptr<ViewRegistry> registry_;
  fidl::BindingSet<mozart::ViewManager, std::unique_ptr<ViewManagerImpl>>
      view_manager_bindings_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ViewManagerApp);
};

}  // namespace view_manager

#endif  // APPS_MOZART_SRC_VIEW_MANAGER_VIEW_MANAGER_APP_H_

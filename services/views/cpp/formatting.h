// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SERVICES_VIEWS_CPP_FORMATTING_H_
#define APPS_MOZART_SERVICES_VIEWS_CPP_FORMATTING_H_

#include <iosfwd>

#include "apps/mozart/services/composition/cpp/formatting.h"
#include "apps/mozart/services/geometry/cpp/formatting.h"
#include "apps/mozart/services/views/view_associates.fidl.h"
#include "apps/mozart/services/views/view_manager.fidl.h"
#include "lib/fidl/cpp/bindings/formatting.h"

namespace mozart {

std::ostream& operator<<(std::ostream& os, const ViewToken& value);

std::ostream& operator<<(std::ostream& os, const ViewTreeToken& value);

std::ostream& operator<<(std::ostream& os, const ViewInfo& value);

std::ostream& operator<<(std::ostream& os, const ViewProperties& value);
std::ostream& operator<<(std::ostream& os, const DisplayMetrics& value);
std::ostream& operator<<(std::ostream& os, const ViewLayout& value);

std::ostream& operator<<(std::ostream& os, const ViewInvalidation& value);

std::ostream& operator<<(std::ostream& os, const ViewAssociateInfo& value);

}  // namespace mozart

#endif  // APPS_MOZART_SERVICES_VIEWS_CPP_FORMATTING_H_

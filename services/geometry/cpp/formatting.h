// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SERVICES_GEOMETRY_CPP_FORMATTING_H_
#define APPS_MOZART_SERVICES_GEOMETRY_CPP_FORMATTING_H_

#include "apps/mozart/services/geometry/geometry.fidl.h"

namespace mozart {

std::ostream& operator<<(std::ostream& os, const Point& value);
std::ostream& operator<<(std::ostream& os, const PointF& value);
std::ostream& operator<<(std::ostream& os, const Rect& value);
std::ostream& operator<<(std::ostream& os, const RectF& value);
std::ostream& operator<<(std::ostream& os, const RRectF& value);
std::ostream& operator<<(std::ostream& os, const Size& value);
std::ostream& operator<<(std::ostream& os, const SizeF& value);
std::ostream& operator<<(std::ostream& os, const Inset& value);
std::ostream& operator<<(std::ostream& os, const InsetF& value);
std::ostream& operator<<(std::ostream& os, const Transform& value);

}  // namespace mozart

#endif  // APPS_MOZART_SERVICES_GEOMETRY_CPP_FORMATTING_H_

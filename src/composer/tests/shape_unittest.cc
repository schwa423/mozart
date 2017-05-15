// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/lib/composer/session_helpers.h"
#include "apps/mozart/src/composer/shapes/circle_shape.h"
#include "apps/mozart/src/composer/tests/session_test.h"

#include "gtest/gtest.h"

namespace mozart {
namespace composer {
namespace test {

typedef SessionTest ShapeTest;

TEST_F(ShapeTest, CircleCreation) {
  ResourceId id = 1;
  EXPECT_TRUE(Apply(NewCreateCircleOp(id, 50.f)));
  auto circle = FindResource<CircleShape>(id);
  ASSERT_NE(nullptr, circle.get());
  EXPECT_EQ(50.f, circle->radius());
}

}  // namespace test
}  // namespace composer
}  // namespace mozart

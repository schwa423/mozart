// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>

#include "apps/mozart/lib/scene/session_helpers.h"
#include "apps/mozart/src/scene/tests/session_test.h"
#include "apps/mozart/src/scene/util/unwrap.h"

#include "gtest/gtest.h"

namespace mozart {
namespace scene {
namespace test {
namespace {
using vec3 = escher::vec3;

constexpr vec3 kDownVector{0.f, 0.f, -1.f};
constexpr vec3 kUpVector{0.f, 0.f, 1.f};
}  // namespace

/*
 * We assume that the various ray-intersection tests for each shape
 * have been covered by other unit tests.  So for ease of testing,
 * we only use rectangles and most use simple downward pointing rays.
 *
 * To make things even easier, the rectangles are all 8x8.
 * The number indicated is the tag id.
 *
 *   +------+  +------+
 *   |      |  |      |   #10 corners: (0, 0, 2) .. (8, 8, 2)
 *   |  10  +--+  30  |   #20 corners: (5, 5, 1) .. (13, 13, 1)
 *   |      |  |      |   #30 corners: (10, 0, 1) .. (18, 8, 1)
 *   +----+-+20+-+----+
 *        |      |
 *        +------+
 *
 * These rectangles are arranged such that their front-to-back drawing
 * order is #30, #20, #10 which forces a tie-break for distance sorting
 * at the intersections of #20 and #30.
 *
 * The node tree looks like this:
 *   @1: entity node
 *   + @2: entity node, tag #100
 *     + @3: entity node, tag #25, translate x+4, y+4
 *       + @4: shape node, tag #10, translate z+2
 *       + @5: entity node, tag #20, translate x+5, y+5, z+1
 *         + @6: shape node
 *     + @7: entity node, tag #35, translate x+10, z+1
 *       + @8: shape node, tag #30, translate x+4, y+4
 *   + @9: entity node, tag #1
 *
 * The 8x8 square used for testing has shape id 20.
 */

class HitTestTest : public SessionTest {
 public:
  void SetUp() override {
    SessionTest::SetUp();

    Apply(NewCreateRectangleOp(20, 8.f, 8.f));

    Apply(NewCreateEntityNodeOp(1));

    Apply(NewCreateEntityNodeOp(2));
    Apply(NewSetTagOp(2, 100));
    Apply(NewAddChildOp(1, 2));

    Apply(NewCreateEntityNodeOp(3));
    Apply(NewSetTagOp(3, 25));
    Apply(NewSetTranslationOp(3, (float[3]){4.f, 4.f, 0.f}));
    Apply(NewAddChildOp(2, 3));

    Apply(NewCreateShapeNodeOp(4));
    Apply(NewSetTagOp(4, 10));
    Apply(NewSetShapeOp(4, 20));
    Apply(NewSetTranslationOp(4, (float[3]){0.f, 0.f, 2.f}));
    Apply(NewAddChildOp(3, 4));

    Apply(NewCreateEntityNodeOp(5));
    Apply(NewSetTagOp(5, 20));
    Apply(NewSetTranslationOp(5, (float[3]){5.f, 5.f, 1.f}));
    Apply(NewAddChildOp(3, 5));

    Apply(NewCreateShapeNodeOp(6));
    Apply(NewSetShapeOp(6, 20));
    Apply(NewAddChildOp(5, 6));

    Apply(NewCreateEntityNodeOp(7));
    Apply(NewSetTagOp(7, 35));
    Apply(NewSetTranslationOp(7, (float[3]){10.f, 0.f, 1.f}));
    Apply(NewAddChildOp(2, 7));

    Apply(NewCreateShapeNodeOp(8));
    Apply(NewSetTagOp(8, 30));
    Apply(NewSetShapeOp(8, 20));
    Apply(NewSetTranslationOp(8, (float[3]){4.f, 4.f, 0.f}));
    Apply(NewAddChildOp(7, 8));

    Apply(NewCreateEntityNodeOp(9));
    Apply(NewSetTagOp(9, 1));
    Apply(NewAddChildOp(1, 9));
  }

 protected:
  struct ExpectedHit {
    uint32_t tag;
    float tx;  // inverse transform translation components
    float ty;
    float tz;
    float d;  // distance
  };

  void ExpectHits(uint32_t node_id,
                  const vec3& ray_origin,
                  const vec3& ray_direction,
                  std::vector<ExpectedHit> expected_hits,
                  bool expected_null = false) {
    mozart2::vec3 wrapped_ray_origin;
    wrapped_ray_origin.x = ray_origin.x;
    wrapped_ray_origin.y = ray_origin.y;
    wrapped_ray_origin.z = ray_origin.z;

    mozart2::vec3 wrapped_ray_direction;
    wrapped_ray_direction.x = ray_direction.x;
    wrapped_ray_direction.y = ray_direction.y;
    wrapped_ray_direction.z = ray_direction.z;

    fidl::Array<mozart2::HitPtr> actual_hits;
    session_->HitTest(node_id, wrapped_ray_origin.Clone(),
                      wrapped_ray_direction.Clone(),
                      [&actual_hits](fidl::Array<mozart2::HitPtr> hits) {
                        actual_hits = std::move(hits);
                      });

    EXPECT_EQ(expected_null, actual_hits.is_null());
    EXPECT_EQ(expected_hits.size(), actual_hits.size());
    for (uint32_t i = 0; i < std::min(expected_hits.size(), actual_hits.size());
         i++) {
      EXPECT_EQ(expected_hits[i].tag, actual_hits[i]->tag_value) << "i=" << i;
      const auto& m = actual_hits[i]->inverse_transform->matrix;
      EXPECT_EQ(1.f, m[0]) << "i=" << i;
      EXPECT_EQ(0.f, m[1]) << "i=" << i;
      EXPECT_EQ(0.f, m[2]) << "i=" << i;
      EXPECT_EQ(0.f, m[3]) << "i=" << i;
      EXPECT_EQ(0.f, m[4]) << "i=" << i;
      EXPECT_EQ(1.f, m[5]) << "i=" << i;
      EXPECT_EQ(0.f, m[6]) << "i=" << i;
      EXPECT_EQ(0.f, m[7]) << "i=" << i;
      EXPECT_EQ(0.f, m[8]) << "i=" << i;
      EXPECT_EQ(0.f, m[9]) << "i=" << i;
      EXPECT_EQ(1.f, m[10]) << "i=" << i;
      EXPECT_EQ(0.f, m[11]) << "i=" << i;
      EXPECT_EQ(expected_hits[i].tx, m[12]) << "i=" << i;
      EXPECT_EQ(expected_hits[i].ty, m[13]) << "i=" << i;
      EXPECT_EQ(expected_hits[i].tz, m[14]) << "i=" << i;
      EXPECT_EQ(1.f, m[15]) << "i=" << i;
      EXPECT_EQ(expected_hits[i].d, actual_hits[i]->distance) << "i=" << i;
    }
  }
};

TEST_F(HitTestTest, InvalidNodeId) {
  ExpectHits(0, vec3(0.f, 0.f, 0.f), kDownVector, {}, true);
}

TEST_F(HitTestTest, RayBelowScenePointingDown) {
  ExpectHits(1, vec3(0.f, 0.f, -10.f), kDownVector, {});
}

TEST_F(HitTestTest, RayBelowScenePointingUp) {
  ExpectHits(1, vec3(0.f, 0.f, -10.f), kUpVector, {});
}

TEST_F(HitTestTest, RayAboveScenePointingUp) {
  ExpectHits(1, vec3(0.f, 0.f, 10.f), kUpVector, {});
}

TEST_F(HitTestTest, Hit10InTopLeftCornerFromNode1) {
  ExpectHits(1, vec3(0.f, 0.f, 10.f), kDownVector,
             {{.tag = 10, .tx = -4.f, .ty = -4.f, .tz = -2.f, .d = 8.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 8.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 8.f}});
}

TEST_F(HitTestTest, Hit10InTopLeftCornerFromNode2) {
  ExpectHits(2, vec3(0.f, 0.f, 10.f), kDownVector,
             {{.tag = 10, .tx = -4.f, .ty = -4.f, .tz = -2.f, .d = 8.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 8.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 8.f}});
}

TEST_F(HitTestTest, Hit10InTopLeftCornerFromNode3) {
  ExpectHits(3, vec3(-4.f, -4.f, 10.f), kDownVector,
             {{.tag = 10, .tx = 0.f, .ty = 0.f, .tz = -2.f, .d = 8.f},
              {.tag = 25, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 8.f}});
}

TEST_F(HitTestTest, Hit10InTopLeftCornerFromNode4) {
  ExpectHits(4, vec3(-4.f, -4.f, 10.f), kDownVector,
             {{.tag = 10, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 10.f}});
}

TEST_F(HitTestTest, Hit20InMiddleFromNode1) {
  ExpectHits(1, vec3(9.f, 9.f, 10.f), kDownVector,
             {{.tag = 20, .tx = -9.f, .ty = -9.f, .tz = -1.f, .d = 9.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 9.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 9.f}});
}

TEST_F(HitTestTest, Hit20InMiddleFromNode2) {
  ExpectHits(2, vec3(9.f, 9.f, 10.f), kDownVector,
             {{.tag = 20, .tx = -9.f, .ty = -9.f, .tz = -1.f, .d = 9.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 9.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 9.f}});
}

TEST_F(HitTestTest, Hit20InMiddleFromNode3) {
  ExpectHits(3, vec3(5.f, 5.f, 10.f), kDownVector,
             {{.tag = 20, .tx = -5.f, .ty = -5.f, .tz = -1.f, .d = 9.f},
              {.tag = 25, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 9.f}});
}

TEST_F(HitTestTest, Hit20InMiddleFromNode5) {
  ExpectHits(5, vec3(0.f, 0.f, 10.f), kDownVector,
             {{.tag = 20, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 10.f}});
}

TEST_F(HitTestTest, Hit20InMiddleFromNode6) {
  ExpectHits(6, vec3(0.f, 0.f, 10.f), kDownVector, {});
}

TEST_F(HitTestTest, HitBoth10And20FromNode1) {
  // 10 is nearer so it comes before 20
  ExpectHits(1, vec3(6.f, 6.f, 10.f), kDownVector,
             {{.tag = 10, .tx = -4.f, .ty = -4.f, .tz = -2.f, .d = 8.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 8.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 8.f},
              {.tag = 20, .tx = -9.f, .ty = -9.f, .tz = -1.f, .d = 9.f}});
}

// TODO(MZ-160): This test is flaky because the traversal order of children
// is unstable.  We should change Node to store its children, parts, and
// imports in a predictable order such as sequentially by order of insertion
// instead of using an std::set keyed by a pointer.
TEST_F(HitTestTest, DISABLED_HitBoth20And30FromNode1) {
  // 20 and 30 are same distance but 30 is closer in draw order so it
  // comes before 20
  ExpectHits(1, vec3(12.f, 6.f, 10.f), kDownVector,
             {{.tag = 20, .tx = -9.f, .ty = -9.f, .tz = -1.f, .d = 9.f},
              {.tag = 25, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 9.f},
              {.tag = 30, .tx = -4.f, .ty = -4.f, .tz = 0.f, .d = 9.f},
              {.tag = 35, .tx = -10.f, .ty = 0.f, .tz = -1.f, .d = 9.f},
              {.tag = 100, .tx = 0.f, .ty = 0.f, .tz = 0.f, .d = 9.f}});
}

}  // namespace test
}  // namespace scene
}  // namespace mozart

// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/composer/session/session.h"
#include "apps/mozart/src/composer/session/session_context.h"

#include "gtest/gtest.h"

namespace mozart {
namespace composer {
namespace test {

class SessionTest : public ::testing::Test,
                    public composer::SessionContext,
                    public composer::ErrorReporter {
 public:
  // ::testing::Test virtual method.
  void SetUp() override;
  void TearDown() override;
  vk::Device vk_device() override { return nullptr; };
  escher::ResourceLifePreserver* escher_resource_life_preserver() override {
    return nullptr;
  };
  escher::ImageFactory* escher_image_factory() override { return nullptr; };
  escher::impl::GpuUploader* escher_gpu_uploader() override { return nullptr; };

 protected:
  // Implement ErrorReporter.
  void ReportError(ftl::LogSeverity severity,
                   std::string error_string) override;

  // Implement SessionContext.
  LinkPtr CreateLink(Session* session,
                     ResourceId id,
                     const mozart2::LinkPtr& args) override;

  void OnSessionTearDown(Session* session) override;

  // Apply the specified Op, and verify that it succeeds.
  bool Apply(const mozart2::OpPtr& op) { return session_->ApplyOp(op); }

  template <class ResourceT>
  ftl::RefPtr<ResourceT> FindResource(ResourceId id) {
    return session_->resources()->FindResource<ResourceT>(id);
  }

  // Verify that the last reported error is as expected.  If no error is
  // expected, use nullptr as |expected_error_string|.
  void ExpectLastReportedError(const char* expected_error_string) {
    if (!expected_error_string) {
      EXPECT_TRUE(reported_errors_.empty());
    } else {
      EXPECT_EQ(reported_errors_.back(), expected_error_string);
    }
  }

  SessionPtr session_;
  std::vector<std::string> reported_errors_;
};

}  // namespace test
}  // namespace composer
}  // namespace mozart

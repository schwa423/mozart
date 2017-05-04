// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_H_
#define APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_H_

#include "apps/mozart/services/input/input_device_registry.fidl.h"
#include "lib/fidl/cpp/bindings/binding.h"
#include "lib/ftl/macros.h"

namespace mozart {
namespace test {

using OnReportCallback = std::function<void(mozart::InputReportPtr report)>;

class MockInputDevice : public mozart::InputDevice {
 public:
  MockInputDevice(
      uint32_t device_id,
      mozart::DeviceDescriptorPtr descriptor,
      fidl::InterfaceRequest<mozart::InputDevice> input_device_request,
      const OnReportCallback& on_report_callback);
  ~MockInputDevice();

  uint32_t id() { return id_; }
  mozart::DeviceDescriptor* descriptor() { return descriptor_.get(); }

  // |InputDevice|
  void DispatchReport(mozart::InputReportPtr report) override;

 private:
  uint32_t id_;
  mozart::DeviceDescriptorPtr descriptor_;
  fidl::Binding<mozart::InputDevice> input_device_binding_;
  OnReportCallback on_report_callback_;

  FTL_DISALLOW_COPY_AND_ASSIGN(MockInputDevice);
};

}  // namespace test
}  // namespace mozart

#endif  // APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_H_

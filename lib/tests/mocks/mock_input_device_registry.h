// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_REGISTRY_H_
#define APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_REGISTRY_H_

#include <memory>
#include <unordered_map>

#include "application/lib/app/application_context.h"
#include "apps/mozart/lib/tests/mocks/mock_input_device.h"
#include "apps/mozart/services/input/input_device_registry.fidl.h"
#include "lib/ftl/macros.h"

namespace mozart {
namespace test {

using OnDeviceCallback = std::function<void(MockInputDevice*)>;

class MockInputDeviceRegistry : public mozart::InputDeviceRegistry {
 public:
  MockInputDeviceRegistry(const OnDeviceCallback on_device_callback,
                          const OnReportCallback on_report_callback);
  ~MockInputDeviceRegistry();

  // |InputDeviceRegistry|:
  void RegisterDevice(mozart::DeviceDescriptorPtr descriptor,
                      fidl::InterfaceRequest<mozart::InputDevice>
                          input_device_request) override;

 private:
  OnDeviceCallback on_device_callback_;
  OnReportCallback on_report_callback_;

  uint32_t next_device_token_ = 0;
  std::unordered_map<uint32_t, std::unique_ptr<MockInputDevice>> devices_by_id_;

  FTL_DISALLOW_COPY_AND_ASSIGN(MockInputDeviceRegistry);
};

}  // namespace test
}  // namespace mozart

#endif  // APPS_MOZART_LIB_TESTS_MOCKS_MOCK_INPUT_DEVICE_REGISTRY_H_

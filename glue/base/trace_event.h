// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_GLUE_BASE_TRACE_EVENT_H_
#define APPS_MOZART_GLUE_BASE_TRACE_EVENT_H_

#if 0
// Stubs to disable tracing for certain development purposes.
#define TRACE_EVENT0(a, b)
#define TRACE_EVENT1(a, b, c, d) (void)d
#define TRACE_EVENT2(a, b, c, d, e, f) (void)d, (void)f
#define TRACE_EVENT_ASYNC_BEGIN0(a, b, c)
#define TRACE_EVENT_ASYNC_END0(a, b, c)
#define TRACE_EVENT_ASYNC_BEGIN1(a, b, c, d, e) (void)e
#define TRACE_EVENT_ASYNC_END1(a, b, c, d, e) (void)e
#define TRACE_EVENT_FLOW_BEGIN0(a, b, c) (void)c
#define TRACE_EVENT_FLOW_END0(a, b, c) (void)c
#define TRACE_EVENT_FLOW_BEGIN1(a, b, c, d, e) (void)c, (void)e
#define TRACE_EVENT_FLOW_END1(a, b, c, d, e) (void)c, (void)e
#define TRACE_EVENT_INSTANT0(a, b, c)
#define TRACE_EVENT_INSTANT1(a, b, c, d, e) (void)e
#define TRACE_EVENT_INSTANT2(a, b, c, d, e, f, g) (void)e, (void)g
#define TRACE_EVENT_SCOPE_GLOBAL
#define TRACE_EVENT_SCOPE_PROCESS
#define TRACE_EVENT_SCOPE_THREAD
#else
#include "apps/mozart/glue/ktrace/trace_event.h"
#endif

#endif  // APPS_MOZART_GLUE_BASE_TRACE_EVENT_H_

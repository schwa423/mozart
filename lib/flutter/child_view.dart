// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:collection';
import 'dart:mozart.internal';

import 'package:apps.modular.lib.app.dart/app.dart';
import 'package:apps.modular.services.application/service_provider.fidl.dart';
import 'package:apps.mozart.services.geometry/geometry.fidl.dart' as fidl;
import 'package:apps.mozart.services.views/view_containers.fidl.dart';
import 'package:apps.mozart.services.views/view_properties.fidl.dart';
import 'package:apps.mozart.services.views/view_provider.fidl.dart';
import 'package:apps.mozart.services.views/view_token.fidl.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:lib.fidl.dart/bindings.dart';
import 'package:lib.fidl.dart/core.dart' as core;

export 'package:apps.mozart.services.views/view_token.fidl.dart' show ViewOwner;

ViewContainerProxy _initViewContainer() {
  final int viewContainerHandle = MozartStartupInfo.takeViewContainer();
  if (viewContainerHandle == null)
    return null;
  final core.Handle handle = new core.Handle(viewContainerHandle);
  final ViewContainerProxy proxy = new ViewContainerProxy()
    ..ctrl.bind(new InterfaceHandle<ViewContainer>(new core.Channel(handle), 0))
    ..setListener(_ViewContainerListenerImpl.instance.createInterfaceHandle());
  return proxy;
}

final ViewContainerProxy _viewContainer = _initViewContainer();

class _ViewContainerListenerImpl extends ViewContainerListener {
  final ViewContainerListenerBinding _binding =
      new ViewContainerListenerBinding();

  InterfaceHandle<ViewContainerListener> createInterfaceHandle() {
    return _binding.wrap(this);
  }

  static final _ViewContainerListenerImpl instance = new _ViewContainerListenerImpl();

  @override
  void onChildAttached(int childKey,ViewInfo childViewInfo,void callback()) {
    ChildViewConnection connection = _connections[childKey];
    connection?._onAttachedToContainer(childViewInfo);
    callback();
  }

  @override
  void onChildUnavailable(int childKey,void callback()) {
    ChildViewConnection connection = _connections[childKey];
    connection?._onUnavailable();
    callback();
  }

  final Map<int, ChildViewConnection> _connections = new HashMap<int, ChildViewConnection>();
}

/// A connection with a child view.
///
/// Used with the [ChildView] widget to display a child view.
class ChildViewConnection {
  ChildViewConnection(this._viewOwner);

  factory ChildViewConnection.connect(ServiceProvider services) {
    final ViewProviderProxy viewProvider = new ViewProviderProxy();
    connectToService(services, viewProvider.ctrl);
    final ViewOwnerProxy viewOwner = new ViewOwnerProxy();
    viewProvider.createView(viewOwner.ctrl.request(), null);
    return new ChildViewConnection(viewOwner.ctrl.unbind());
  }

  InterfaceHandle<ViewOwner> _viewOwner;

  static int _nextViewKey = 1;
  int _viewKey;

  int _sceneVersion = 1;
  ViewProperties _currentViewProperties;

  VoidCallback _onViewInfoAvailable;
  ViewInfo _viewInfo;

  void _onAttachedToContainer(ViewInfo viewInfo) {
    assert(_viewInfo == null);
    _viewInfo = viewInfo;
    if (_onViewInfoAvailable != null)
      _onViewInfoAvailable();
  }

  void _onUnavailable() {
    _viewInfo = null;
  }

  void _addChildToViewHost() {
    if (_viewContainer == null)
      return;
    assert(_attached);
    assert(_viewOwner != null);
    assert(_viewKey == null);
    assert(_viewInfo == null);
    _viewKey = _nextViewKey++;
    _viewContainer.addChild(_viewKey, _viewOwner);
    _viewOwner = null;
    assert(!_ViewContainerListenerImpl.instance._connections.containsKey(_viewKey));
    _ViewContainerListenerImpl.instance._connections[_viewKey] = this;
  }

  void _removeChildFromViewHost() {
    if (_viewContainer == null)
      return;
    assert(!_attached);
    assert(_viewOwner == null);
    assert(_viewKey != null);
    assert(_ViewContainerListenerImpl.instance._connections[_viewKey] == this);
    final core.ChannelPair pair = new core.ChannelPair();
    _ViewContainerListenerImpl.instance._connections.remove(_viewKey);
    _viewOwner = new InterfaceHandle<ViewOwner>(pair.channel0, 0);
    _viewContainer.removeChild(_viewKey, new InterfaceRequest<ViewOwner>(pair.channel1));
    _viewKey = null;
    _viewInfo = null;
    _currentViewProperties = null;
  }

  // The number of render objects attached to this view. In between frames, we
  // might have more than one connected if we get added to a new render object
  // before we get removed from the old render object. By the time we get around
  // to computing our layout, we must be back to just having one render object.
  int _attachments = 0;
  bool get _attached => _attachments > 0;

  void _attach() {
    assert(_attachments >= 0);
    ++_attachments;
    if (_viewKey == null)
      _addChildToViewHost();
  }

  void _detach() {
    assert(_attached);
    --_attachments;
    scheduleMicrotask(_removeChildFromViewHostIfNeeded);
  }

  void _removeChildFromViewHostIfNeeded() {
    assert(_attachments >= 0);
    if (_attachments == 0)
      _removeChildFromViewHost();
  }

  ViewProperties _createViewProperties(int physicalWidth,
                                       int physicalHeight,
                                       double devicePixelRatio) {
    if (_currentViewProperties != null &&
        _currentViewProperties.displayMetrics.devicePixelRatio == devicePixelRatio &&
        _currentViewProperties.viewLayout.size.width == physicalWidth &&
        _currentViewProperties.viewLayout.size.height == physicalHeight)
      return null;

    DisplayMetrics displayMetrics = new DisplayMetrics()
      ..devicePixelRatio = devicePixelRatio;
    fidl.Size size = new fidl.Size()
      ..width = physicalWidth
      ..height = physicalHeight;
    ViewLayout viewLayout = new ViewLayout()
      ..size = size;
    _currentViewProperties = new ViewProperties()
      ..displayMetrics = displayMetrics
      ..viewLayout = viewLayout;
    return _currentViewProperties;
  }

  void _setChildProperties(int physicalWidth, int physicalHeight, double devicePixelRatio) {
    assert(_attached);
    assert(_attachments == 1);
    assert(_viewKey != null);
    if (_viewContainer == null)
      return;
    ViewProperties viewProperties = _createViewProperties(physicalWidth, physicalHeight, devicePixelRatio);
    if (viewProperties == null)
      return;
    _viewContainer.setChildProperties(_viewKey, _sceneVersion++, viewProperties);
  }
}

class _RenderChildView extends RenderBox {
  /// Creates a child view render object.
  ///
  /// The [scale] argument must not be null.
  _RenderChildView({
    ChildViewConnection child,
    double scale,
  }) : _scale = scale {
    assert(scale != null);
    this.child = child;
  }

  /// The child to display.
  ChildViewConnection get child => _child;
  ChildViewConnection _child;
  set child (ChildViewConnection value) {
    if (value == _child)
      return;
    if (attached && _child != null) {
      _child._detach();
      assert(_child._onViewInfoAvailable != null);
      _child._onViewInfoAvailable = null;
    }
    _child = value;
    if (attached && _child != null) {
      _child._attach();
      assert(_child._onViewInfoAvailable == null);
      _child._onViewInfoAvailable = markNeedsPaint;
    }
    if (_child == null) {
      markNeedsPaint();
    } else {
      markNeedsLayout();
    }
  }

  /// The device pixel ratio to provide the child.
  double get scale => _scale;
  double _scale;
  set scale (double value) {
    assert(value != null);
    if (value == _scale)
      return;
    _scale = value;
    if (_child != null)
      markNeedsLayout();
  }

  @override
  void attach(PipelineOwner owner) {
    super.attach(owner);
    if (_child != null) {
      _child._attach();
      assert(_child._onViewInfoAvailable == null);
      _child._onViewInfoAvailable = markNeedsPaint;
    }
  }

  @override
  void detach() {
    if (_child != null) {
      _child._detach();
      assert(_child._onViewInfoAvailable != null);
      _child._onViewInfoAvailable = null;
    }
    super.detach();
  }

  @override
  bool get alwaysNeedsCompositing => true;

  TextPainter _debugErrorMessage;

  int _physicalWidth;
  int _physicalHeight;

  @override
  void performLayout() {
    size = constraints.biggest;
    if (_child != null) {
      _physicalWidth = (size.width * scale).round();
      _physicalHeight = (size.height * scale).round();
      _child._setChildProperties(_physicalWidth, _physicalHeight, scale);
      assert(() {
        if (_viewContainer == null) {
          _debugErrorMessage ??= new TextPainter(
            text: new TextSpan(text: 'Child views are supported only when running in Mozart.')
          );
          _debugErrorMessage.layout(minWidth: size.width, maxWidth: size.width);
        }
        return true;
      });
    }
  }

  @override
  bool hitTestSelf(Point position) => true;

  @override
  void paint(PaintingContext context, Offset offset) {
    assert(needsCompositing);
    if (_child?._viewInfo != null) {
      context.addLayer(new ChildSceneLayer(
        offset: offset,
        devicePixelRatio: scale,
        physicalWidth: _physicalWidth,
        physicalHeight: _physicalHeight,
        sceneToken: _child._viewInfo.sceneToken.value
      ));
    }
    assert(() {
      if (_viewContainer == null) {
        context.canvas.drawRect(offset & size, new Paint()..color = const Color(0xFF0000FF));
        _debugErrorMessage.paint(context.canvas, offset);
      }
      return true;
    });
  }

  @override
  void debugFillDescription(List<String> description) {
    super.debugFillDescription(description);
    description.add('child: $child');
    description.add('scale: $scale');
  }
}

/// A widget that is replaced by content from another process.
///
/// Requires a [MediaQuery] ancestor to provide appropriate media information to
/// the child.
class ChildView extends LeafRenderObjectWidget {
  /// Creates a widget that is replaced by content from another process.
  ChildView({ ChildViewConnection child })
    : child = child, super(key: new GlobalObjectKey(child));

  /// A connection to the child whose content will replace this widget.
  final ChildViewConnection child;

  @override
  _RenderChildView createRenderObject(BuildContext context) {
    return new _RenderChildView(
      child: child,
      scale: MediaQuery.of(context).devicePixelRatio
    );
  }

  @override
  void updateRenderObject(BuildContext context, _RenderChildView renderObject) {
    renderObject
      ..child = child
      ..scale = MediaQuery.of(context).devicePixelRatio;
  }
}

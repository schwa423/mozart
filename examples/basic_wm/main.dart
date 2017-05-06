// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:application.lib.app.dart/app.dart';
import 'package:application.services/application_controller.fidl.dart';
import 'package:application.services/application_environment.fidl.dart';
import 'package:application.services/application_environment_host.fidl.dart';
import 'package:application.services/application_launcher.fidl.dart';
import 'package:application.services/service_provider.fidl.dart';
import 'package:apps.mozart.lib.flutter/child_view.dart';
import 'package:apps.mozart.services.presentation/presenter.fidl.dart';
import 'package:lib.fidl.dart/bindings.dart';
import 'package:flutter/material.dart';

import 'window_manager.dart';

final ApplicationContext _context = new ApplicationContext.fromStartupInfo();

final ApplicationEnvironmentProxy _childEnvironment = _initChildEnvironment();
final ApplicationLauncherProxy _childLauncher = _initLauncher();

class ChildApplication {
  ChildApplication({
    this.controller,
    this.connection,
    this.title,
  });

  factory ChildApplication.create(String url, {String title}) {
    final ApplicationControllerProxy controller =
        new ApplicationControllerProxy();
    return new ChildApplication(
      controller: controller,
      connection: new ChildViewConnection.launch(url, _childLauncher,
          controller: controller.ctrl.request()),
      title: title,
    );
  }

  factory ChildApplication.view(InterfaceHandle<ViewOwner> viewOwner) {
    return new ChildApplication(
      connection: new ChildViewConnection(viewOwner),
    );
  }

  void close() {
    if (controller != null) {
      controller.kill();
      controller.ctrl.close();
    }
  }

  final ApplicationControllerProxy controller;
  final ChildViewConnection connection;
  final String title;
}

final GlobalKey<WindowManagerState> _windowManager =
    new GlobalKey<WindowManagerState>();

void addWindowForChildApplication(ChildApplication application) {
  _windowManager.currentState.addWindow(
      new WindowFrame(
        key: new ObjectKey(application),
        title: application.title,
        child: new ChildView(connection: application.connection),
      ), onClose: () {
    application.close();
  });
}

class PresenterImpl extends Presenter {
  final PresenterBinding _binding = new PresenterBinding();

  void bind(InterfaceRequest<Presenter> request) {
    _binding.bind(this, request);
  }

  @override
  void present(InterfaceHandle<ViewOwner> viewOwner) {
    addWindowForChildApplication(new ChildApplication.view(viewOwner));
  }
}

class ApplicationEnvironmentHostImpl extends ApplicationEnvironmentHost {
  final ApplicationEnvironmentHostBinding _binding =
      new ApplicationEnvironmentHostBinding();

  void bind(InterfaceRequest<ApplicationEnvironmentHost> request) {
    _binding.bind(this, request);
  }

  InterfaceHandle<ApplicationEnvironmentHost> getInterfaceHandle() {
    return _binding.wrap(this);
  }

  @override
  void getApplicationEnvironmentServices(
      InterfaceRequest<ServiceProvider> services) {
    ServiceProviderImpl impl = new ServiceProviderImpl()
      ..bind(services)
      ..addServiceForName((request) {
        new PresenterImpl().bind(request);
      }, Presenter.serviceName)
      ..defaultConnector = (String serviceName, InterfaceRequest request) {
        _context.environmentServices
            .connectToService(serviceName, request.passChannel());
      };
    // TODO(abarth): Add a proper BindingSet to the FIDL Dart bindings so we
    // accumulate all these service provider impls.
    _serviceProviders.add(impl);
  }

  final List<ServiceProviderImpl> _serviceProviders = <ServiceProviderImpl>[];
}

final ApplicationEnvironmentHostImpl _environmentHost =
    new ApplicationEnvironmentHostImpl();

ApplicationEnvironmentProxy _initChildEnvironment() {
  final ApplicationEnvironmentProxy proxy = new ApplicationEnvironmentProxy();
  _context.environment.createNestedEnvironment(
    _environmentHost.getInterfaceHandle(),
    proxy.ctrl.request(),
    null,
    'basic_wm',
  );
  return proxy;
}

ApplicationLauncherProxy _initLauncher() {
  final ApplicationLauncherProxy proxy = new ApplicationLauncherProxy();
  _childEnvironment.getApplicationLauncher(proxy.ctrl.request());
  return proxy;
}

class App extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return new Theme(
      data: new ThemeData.light(),
      child: new WindowManager(
        key: _windowManager,
        wallpaper: new Container(
          decoration: new BoxDecoration(
            gradient: new LinearGradient(
              begin: FractionalOffset.topLeft,
              end: FractionalOffset.bottomRight,
              colors: <Color>[Colors.green[200], Colors.green[400]],
            ),
          ),
        ),
        initialWindows: <Widget>[
          new Positioned(
            top: 24.0,
            right: 24.0,
            child: new FlutterLogo(
              size: 48.0,
            ),
          ),
          new Positioned(
            right: 24.0,
            bottom: 24.0,
            child: new FloatingActionButton(
              onPressed: () {
                addWindowForChildApplication(new ChildApplication.create(
                  'file:///system/apps/moterm',
                  title: 'Terminal',
                ));
              },
              child: new Icon(Icons.apps),
            ),
          )
        ],
      ),
    );
  }
}

void main() {
  runApp(new App());
}

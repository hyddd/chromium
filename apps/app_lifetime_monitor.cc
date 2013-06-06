// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/app_lifetime_monitor.h"

#include "chrome/browser/extensions/extension_host.h"
#include "chrome/browser/ui/extensions/shell_window.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/extensions/extension.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"

namespace apps {

using extensions::Extension;
using extensions::ExtensionHost;
using extensions::ShellWindowRegistry;

AppLifetimeMonitor::AppLifetimeMonitor(Profile* profile)
    : profile_(profile) {
  registrar_.Add(
      this, chrome::NOTIFICATION_EXTENSION_HOST_DID_STOP_LOADING,
      content::NotificationService::AllSources());
  registrar_.Add(
      this, chrome::NOTIFICATION_EXTENSION_HOST_DESTROYED,
      content::NotificationService::AllSources());

  ShellWindowRegistry* shell_window_registry =
      ShellWindowRegistry::Factory::GetForProfile(
          profile_, false /* create */);
  if (shell_window_registry)
    shell_window_registry->AddObserver(this);
}

AppLifetimeMonitor::~AppLifetimeMonitor() {}

void AppLifetimeMonitor::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void AppLifetimeMonitor::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void AppLifetimeMonitor::Observe(int type,
                                const content::NotificationSource& source,
                                const content::NotificationDetails& details) {
  ExtensionHost* host = content::Details<ExtensionHost>(details).ptr();
  const Extension* extension = host->extension();
  if (!extension || !extension->is_platform_app())
    return;

  switch (type) {
    case chrome::NOTIFICATION_EXTENSION_HOST_DID_STOP_LOADING: {
      NotifyAppStart(extension->id());
      break;
    }

    case chrome::NOTIFICATION_EXTENSION_HOST_DESTROYED: {
      NotifyAppStop(extension->id());
      break;
    }
  }
}

void AppLifetimeMonitor::OnShellWindowAdded(ShellWindow* shell_window) {
  if (shell_window->window_type() != ShellWindow::WINDOW_TYPE_DEFAULT)
    return;

  ShellWindowRegistry::ShellWindowList windows =
      ShellWindowRegistry::Get(shell_window->profile())->
          GetShellWindowsForApp(shell_window->extension_id());
  if (windows.size() == 1)
    NotifyAppActivated(shell_window->extension_id());
}

void AppLifetimeMonitor::OnShellWindowIconChanged(ShellWindow* shell_window) {}

void AppLifetimeMonitor::OnShellWindowRemoved(ShellWindow* shell_window) {
  ShellWindow* window =
      ShellWindowRegistry::Get(shell_window->profile())->
          GetCurrentShellWindowForApp(shell_window->extension_id());
  if (!window)
    NotifyAppDeactivated(shell_window->extension_id());
}

void AppLifetimeMonitor::Shutdown() {
  ShellWindowRegistry* shell_window_registry =
      ShellWindowRegistry::Factory::GetForProfile(
          profile_, false /* create */);
  if (shell_window_registry)
    shell_window_registry->RemoveObserver(this);
}

void AppLifetimeMonitor::NotifyAppStart(const std::string& app_id) {
  FOR_EACH_OBSERVER(Observer, observers_, OnAppStart(profile_, app_id));
}

void AppLifetimeMonitor::NotifyAppActivated(const std::string& app_id) {
  FOR_EACH_OBSERVER(Observer, observers_, OnAppActivated(profile_, app_id));
}

void AppLifetimeMonitor::NotifyAppDeactivated(const std::string& app_id) {
  FOR_EACH_OBSERVER(Observer, observers_, OnAppDeactivated(profile_, app_id));
}

void AppLifetimeMonitor::NotifyAppStop(const std::string& app_id) {
  FOR_EACH_OBSERVER(Observer, observers_, OnAppStop(profile_, app_id));
}

}  // namespace apps

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_tab_restore_service_delegate.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_tabrestore.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/navigation_controller.h"

using content::NavigationController;
using content::SessionStorageNamespace;
using content::WebContents;

void BrowserTabRestoreServiceDelegate::ShowBrowserWindow() {
  browser_->window()->Show();
}

const SessionID& BrowserTabRestoreServiceDelegate::GetSessionID() const {
  return browser_->session_id();
}

int BrowserTabRestoreServiceDelegate::GetTabCount() const {
  return browser_->tab_count();
}

int BrowserTabRestoreServiceDelegate::GetSelectedIndex() const {
  return browser_->active_index();
}

std::string BrowserTabRestoreServiceDelegate::GetAppName() const {
  return browser_->app_name();
}

WebContents* BrowserTabRestoreServiceDelegate::GetWebContentsAt(
    int index) const {
  return chrome::GetWebContentsAt(browser_, index);
}

WebContents* BrowserTabRestoreServiceDelegate::GetActiveWebContents() const {
  return chrome::GetActiveWebContents(browser_);
}

bool BrowserTabRestoreServiceDelegate::IsTabPinned(int index) const {
  return browser_->tab_strip_model()->IsTabPinned(index);
}

WebContents* BrowserTabRestoreServiceDelegate::AddRestoredTab(
      const std::vector<TabNavigation>& navigations,
      int tab_index,
      int selected_navigation,
      const std::string& extension_app_id,
      bool select,
      bool pin,
      bool from_last_session,
      SessionStorageNamespace* storage_namespace) {
  return chrome::AddRestoredTab(browser_, navigations, tab_index,
                                selected_navigation, extension_app_id, select,
                                pin, from_last_session, storage_namespace);
}

void BrowserTabRestoreServiceDelegate::ReplaceRestoredTab(
      const std::vector<TabNavigation>& navigations,
      int selected_navigation,
      bool from_last_session,
      const std::string& extension_app_id,
      SessionStorageNamespace* session_storage_namespace) {
  chrome::ReplaceRestoredTab(browser_, navigations, selected_navigation,
                             from_last_session, extension_app_id,
                             session_storage_namespace);
}

void BrowserTabRestoreServiceDelegate::CloseTab() {
  chrome::CloseTab(browser_);
}

// Implementations of TabRestoreServiceDelegate static methods

// static
TabRestoreServiceDelegate* TabRestoreServiceDelegate::Create(
    Profile* profile,
    const std::string& app_name) {
  Browser* browser;
  if (app_name.empty()) {
    browser = Browser::Create(profile);
  } else {
    browser = Browser::CreateWithParams(
        Browser::CreateParams::CreateForApp(
            Browser::TYPE_POPUP, app_name, gfx::Rect(), profile));
  }
  if (browser)
    return browser->tab_restore_service_delegate();
  else
    return NULL;
}

// static
TabRestoreServiceDelegate*
    TabRestoreServiceDelegate::FindDelegateForWebContents(
        const WebContents* contents) {
  Browser* browser = browser::FindBrowserWithWebContents(contents);
  return browser ? browser->tab_restore_service_delegate() : NULL;
}

// static
TabRestoreServiceDelegate* TabRestoreServiceDelegate::FindDelegateWithID(
    SessionID::id_type desired_id) {
  Browser* browser = browser::FindBrowserWithID(desired_id);
  return browser ? browser->tab_restore_service_delegate() : NULL;
}

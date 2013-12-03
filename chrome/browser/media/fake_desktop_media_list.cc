// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/fake_desktop_media_list.h"

#include "chrome/browser/media/desktop_media_list_observer.h"
#include "ui/gfx/skia_util.h"

FakeDesktopMediaList::FakeDesktopMediaList() : observer_(NULL) {}
FakeDesktopMediaList::~FakeDesktopMediaList() {}

void FakeDesktopMediaList::AddSource(int id) {
  Source source;
  source.id = content::DesktopMediaID(content::DesktopMediaID::TYPE_WINDOW, id);
  source.name = base::Int64ToString16(id);

  sources_.push_back(source);
  observer_->OnSourceAdded(sources_.size() - 1);
}

void FakeDesktopMediaList::RemoveSource(int index) {
  sources_.erase(sources_.begin() + index);
  observer_->OnSourceRemoved(sources_.size() - 1);
}

void FakeDesktopMediaList::SetSourceThumbnail(int index) {
  sources_[index].thumbnail = thumbnail_;
  observer_->OnSourceThumbnailChanged(index);
}

void FakeDesktopMediaList::SetSourceName(int index, string16 name) {
  sources_[index].name = name;
  observer_->OnSourceNameChanged(index);
}

void FakeDesktopMediaList::SetUpdatePeriod(base::TimeDelta period) {}

void FakeDesktopMediaList::SetThumbnailSize(const gfx::Size& thumbnail_size) {}

void FakeDesktopMediaList::SetViewDialogWindowId(
    content::DesktopMediaID::Id dialog_id) {}

void FakeDesktopMediaList::StartUpdating(DesktopMediaListObserver* observer) {
  observer_ = observer;

  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, 150, 150);
  bitmap.allocPixels();
  bitmap.eraseRGB(0, 255, 0);
  thumbnail_ = gfx::ImageSkia::CreateFrom1xBitmap(bitmap);
}

int FakeDesktopMediaList::GetSourceCount() const { return sources_.size(); }

const DesktopMediaList::Source& FakeDesktopMediaList::GetSource(
    int index) const {
  return sources_[index];
}

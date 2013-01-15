// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/bookmarks/bookmark_storage.h"

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/file_util_proxy.h"
#include "base/json/json_file_value_serializer.h"
#include "base/json/json_string_value_serializer.h"
#include "base/metrics/histogram.h"
#include "base/time.h"
#include "chrome/browser/bookmarks/bookmark_codec.h"
#include "chrome/browser/bookmarks/bookmark_model.h"
#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_source.h"

using base::TimeTicks;
using content::BrowserThread;

namespace {

// Extension used for backup files (copy of main file created during startup).
const FilePath::CharType kBackupExtension[] = FILE_PATH_LITERAL("bak");

// How often we save.
const int kSaveDelayMS = 2500;

void BackupCallback(const FilePath& path) {
  FilePath backup_path = path.ReplaceExtension(kBackupExtension);
  file_util::CopyFile(path, backup_path);
}

// Adds node to the model's index, recursing through all children as well.
void AddBookmarksToIndex(BookmarkLoadDetails* details,
                         BookmarkNode* node) {
  if (node->is_url()) {
    if (node->url().is_valid())
      details->index()->Add(node);
  } else {
    for (int i = 0; i < node->child_count(); ++i)
      AddBookmarksToIndex(details, node->GetChild(i));
  }
}

void LoadCallback(const FilePath& path,
                  BookmarkStorage* storage,
                  BookmarkLoadDetails* details) {
  bool bookmark_file_exists = file_util::PathExists(path);
  if (bookmark_file_exists) {
    JSONFileValueSerializer serializer(path);
    scoped_ptr<Value> root(serializer.Deserialize(NULL, NULL));

    if (root.get()) {
      // Building the index can take a while, so we do it on the background
      // thread.
      int64 max_node_id = 0;
      BookmarkCodec codec;
      TimeTicks start_time = TimeTicks::Now();
      codec.Decode(details->bb_node(), details->other_folder_node(),
                   details->mobile_folder_node(), &max_node_id, *root.get());
      details->set_max_id(std::max(max_node_id, details->max_id()));
      details->set_computed_checksum(codec.computed_checksum());
      details->set_stored_checksum(codec.stored_checksum());
      details->set_ids_reassigned(codec.ids_reassigned());
      UMA_HISTOGRAM_TIMES("Bookmarks.DecodeTime",
                          TimeTicks::Now() - start_time);

      start_time = TimeTicks::Now();
      AddBookmarksToIndex(details, details->bb_node());
      AddBookmarksToIndex(details, details->other_folder_node());
      AddBookmarksToIndex(details, details->mobile_folder_node());
      UMA_HISTOGRAM_TIMES("Bookmarks.CreateBookmarkIndexTime",
                          TimeTicks::Now() - start_time);
    }
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&BookmarkStorage::OnLoadFinished, storage,
                 bookmark_file_exists, path));
}

}  // namespace

// BookmarkLoadDetails ---------------------------------------------------------

BookmarkLoadDetails::BookmarkLoadDetails(
    BookmarkPermanentNode* bb_node,
    BookmarkPermanentNode* other_folder_node,
    BookmarkPermanentNode* mobile_folder_node,
    BookmarkIndex* index,
    int64 max_id)
    : bb_node_(bb_node),
      other_folder_node_(other_folder_node),
      mobile_folder_node_(mobile_folder_node),
      index_(index),
      max_id_(max_id),
      ids_reassigned_(false) {
}

BookmarkLoadDetails::~BookmarkLoadDetails() {
}

// BookmarkStorage -------------------------------------------------------------

BookmarkStorage::BookmarkStorage(Profile* profile, BookmarkModel* model)
    : profile_(profile),
      model_(model),
      writer_(profile->GetPath().Append(chrome::kBookmarksFileName),
              BrowserThread::GetMessageLoopProxyForThread(BrowserThread::FILE)),
      tmp_history_path_(
          profile->GetPath().Append(chrome::kHistoryBookmarksFileName)) {
  writer_.set_commit_interval(base::TimeDelta::FromMilliseconds(kSaveDelayMS));
  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
                          base::Bind(&BackupCallback, writer_.path()));
}

BookmarkStorage::~BookmarkStorage() {
  if (writer_.HasPendingWrite())
    writer_.DoScheduledWrite();
}

void BookmarkStorage::LoadBookmarks(BookmarkLoadDetails* details) {
  DCHECK(!details_.get());
  DCHECK(details);
  details_.reset(details);
  DoLoadBookmarks(writer_.path());
}

void BookmarkStorage::DoLoadBookmarks(const FilePath& path) {
  BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(&LoadCallback, path, make_scoped_refptr(this),
                 details_.get()));
}

void BookmarkStorage::MigrateFromHistory() {
  // We need to wait until history has finished loading before reading
  // from generated bookmarks file.
  HistoryService* history =
      HistoryServiceFactory::GetForProfile(profile_, Profile::EXPLICIT_ACCESS);
  if (!history) {
    // This happens in unit tests.
    if (model_)
      model_->DoneLoading(details_.release());
    return;
  }
  if (!history->BackendLoaded()) {
    // The backend isn't finished loading. Wait for it.
    notification_registrar_.Add(this, chrome::NOTIFICATION_HISTORY_LOADED,
                                content::Source<Profile>(profile_));
  } else {
    DoLoadBookmarks(tmp_history_path_);
  }
}

void BookmarkStorage::OnHistoryFinishedWriting() {
  notification_registrar_.Remove(this, chrome::NOTIFICATION_HISTORY_LOADED,
                                 content::Source<Profile>(profile_));

  // This is used when migrating bookmarks data from database to file.
  // History wrote the file for us, and now we want to load data from it.
  DoLoadBookmarks(tmp_history_path_);
}

void BookmarkStorage::ScheduleSave() {
  writer_.ScheduleWrite(this);
}

void BookmarkStorage::BookmarkModelDeleted() {
  // We need to save now as otherwise by the time SaveNow is invoked
  // the model is gone.
  if (writer_.HasPendingWrite())
    SaveNow();
  model_ = NULL;
}

bool BookmarkStorage::SerializeData(std::string* output) {
  BookmarkCodec codec;
  scoped_ptr<Value> value(codec.Encode(model_));
  JSONStringValueSerializer serializer(output);
  serializer.set_pretty_print(true);
  return serializer.Serialize(*(value.get()));
}

void BookmarkStorage::OnLoadFinished(bool file_exists, const FilePath& path) {
  if (path == writer_.path() && !file_exists) {
    // The file doesn't exist. This means one of two things:
    // 1. A clean profile.
    // 2. The user is migrating from an older version where bookmarks were
    //    saved in history.
    // We assume step 2. If history had the bookmarks, it will write the
    // bookmarks to a file for us.
    MigrateFromHistory();
    return;
  }

  if (!model_)
    return;

  model_->DoneLoading(details_.release());

  if (path == tmp_history_path_) {
    // We just finished migration from history. Save now to new file,
    // after the model is created and done loading.
    SaveNow();

    // Clean up after migration from history.
    base::FileUtilProxy::Delete(
        BrowserThread::GetMessageLoopProxyForThread(BrowserThread::FILE),
        tmp_history_path_, false, base::FileUtilProxy::StatusCallback());
  }
}

void BookmarkStorage::Observe(int type,
                              const content::NotificationSource& source,
                              const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_HISTORY_LOADED:
      OnHistoryFinishedWriting();
      break;

    default:
      NOTREACHED();
      break;
  }
}

bool BookmarkStorage::SaveNow() {
  if (!model_ || !model_->IsLoaded()) {
    // We should only get here if we have a valid model and it's finished
    // loading.
    NOTREACHED();
    return false;
  }

  std::string data;
  if (!SerializeData(&data))
    return false;
  writer_.WriteNow(data);
  return true;
}
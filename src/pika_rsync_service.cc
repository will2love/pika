// Copyright (c) 2019-present, Qihoo, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include "include/pika_rsync_service.h"

#include <glog/logging.h>

#include "slash/include/env.h"
#include "slash/include/rsync.h"

#include "include/pika_define.h"

PikaRsyncService::PikaRsyncService(const std::string& raw_path,
                                   const int port)
    : raw_path_(raw_path), port_(port) {
  if (raw_path_.back() != '/') {
    raw_path_ += "/";
  }
  rsync_path_ = raw_path_ + slash::kRsyncSubDir + "/";
  pid_path_ = rsync_path_ + slash::kRsyncPidFile;
}

PikaRsyncService::~PikaRsyncService() {
  if (!CheckRsyncAlive()) {
    slash::DeleteDirIfExist(rsync_path_);
  } else {
    slash::StopRsync(raw_path_);
  }
  LOG(INFO) << "PikaRsyncService exit!!!";
}

int PikaRsyncService::StartRsync() {
  int ret = 0;
  ret = slash::StartRsync(raw_path_, kDBSyncModule, "0.0.0.0", port_);
  if (ret != 0) {
    LOG(WARNING) << "Failed to start rsync, path:" << raw_path_ << " error : " << ret;
    return -1;
  }

  // Make sure the listening addr of rsyncd is accessible, avoid the corner case
  // that rsync --daemon process is started but not finished listening on the socket
  sleep(1);

  if (!CheckRsyncAlive()) {
    LOG(WARNING) << "Rsync service is no live, path:" << raw_path_;
    return -1;
  }
  return 0;
}

bool PikaRsyncService::CheckRsyncAlive() {
  return slash::FileExists(pid_path_);
}

int PikaRsyncService::ListenPort() {
  return port_;
}
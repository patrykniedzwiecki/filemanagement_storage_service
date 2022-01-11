/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef STORAGE_DAEMON_UTILS_FILE_UTILS_H
#define STORAGE_DAEMON_UTILS_FILE_UTILS_H

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/mount.h>

namespace OHOS {
namespace StorageDaemon {
struct FileList {
    uint32_t userId;
    std::string path;
};

int32_t MkDir(const std::string &path, mode_t mode);
bool IsDir(const std::string &path);
bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid);
bool DestroyDir(const std::string &path);
bool RmDirRecurse(const std::string &path);
int32_t Mount(const std::string &source, const std::string &target, const char *type,
              unsigned long flags, const void *data);
int32_t UMount(const std::string &path);
void ReadDigitDir(const std::string &path, std::vector<FileList> &dirInfo);
bool StringToUint32(const std::string &str, uint32_t &num);
}
}

#endif // STORAGE_DAEMON_UTILS_FILE_UTILS_H
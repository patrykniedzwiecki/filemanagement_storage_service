/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_manager.h"

#include <string>

#include "directory_ex.h"
#include "file_ex.h"
#include "utils/log.h"
#include "utils/errno.h"
#include "key_ctrl.h"

namespace OHOS {
namespace StorageDaemon {
const UserAuth NULL_KEY_AUTH = {
    .token = ""
};
const std::string DATA_EL0_DIR = std::string() + "/data/service/el0";
const std::string STORAGE_DAEMON_DIR = DATA_EL0_DIR + "/storage_daemon";
const std::string DEVICE_EL1_DIR = STORAGE_DAEMON_DIR + "/sd";

const std::string FSCRYPT_USER_EL1_PUBLIC = std::string() + "/data/service/el1/public";
const std::string SERVICE_STORAGE_DAEMON_DIR = FSCRYPT_USER_EL1_PUBLIC + "/storage_daemon";
const std::string FSCRYPT_EL_DIR = SERVICE_STORAGE_DAEMON_DIR + "/sd";
const std::string USER_EL1_DIR = FSCRYPT_EL_DIR + "/el1";
const std::string USER_EL2_DIR = FSCRYPT_EL_DIR + "/el2";

int KeyManager::GenerateAndInstallDeviceKey(const std::string &dir)
{
    LOGI("enter");
    globalEl1Key_ = std::make_shared<BaseKey>(dir);
    if (globalEl1Key_ == nullptr) {
        LOGE("No memory for device el1 key");
        return -ENOMEM;
    }

    if (globalEl1Key_->InitKey(FSCRYPT_V2) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (globalEl1Key_->StoreKey(NULL_KEY_AUTH) == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (globalEl1Key_->ActiveKey() == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        return -EFAULT;
    }
    hasGlobalDeviceKey_ = true;
    LOGI("key create success");

    return 0;
}

int KeyManager::RestoreDeviceKey(const std::string &dir)
{
    LOGI("enter");
    if (globalEl1Key_ != nullptr) {
        LOGD("device key has existed");
        return 0;
    }

    globalEl1Key_ = std::make_shared<BaseKey>(dir);
    if (globalEl1Key_ == nullptr) {
        LOGE("No memory for device el1 key");
        return -ENOMEM;
    }

    if (globalEl1Key_->InitKey(FSCRYPT_V2) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (globalEl1Key_->RestoreKey(NULL_KEY_AUTH) == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (globalEl1Key_->ActiveKey() == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        return -EFAULT;
    }
    hasGlobalDeviceKey_ = true;
    LOGI("key restore success");

    return 0;
}

int KeyManager::InitGlobalDeviceKey(void)
{
    LOGI("enter");
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (hasGlobalDeviceKey_ || globalEl1Key_ != nullptr) {
        LOGD("glabal device el1 have existed");
        return 0;
    }

    int ret = MkDir(STORAGE_DAEMON_DIR, 0700);
    if (ret && errno != EEXIST) {
        LOGE("create storage daemon dir error");
        return ret;
    }
    ret = MkDir(DEVICE_EL1_DIR, 0700);
    if (ret) {
        if (errno != EEXIST) {
            LOGE("make device el1 dir error");
            return ret;
        }
        return RestoreDeviceKey(DEVICE_EL1_DIR);
    }

    return GenerateAndInstallDeviceKey(DEVICE_EL1_DIR);
}

int KeyManager::GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("enter");
    if (HasElkey(userId, type)) {
        LOGD("The user %{public}u el %{public}u have existed", userId, type);
        return 0;
    }

    std::shared_ptr<BaseKey> elKey = std::make_shared<BaseKey>(dir);
    if (elKey == nullptr) {
        LOGE("No memory for device el1 key");
        return -ENOMEM;
    }

    if (elKey->InitKey(FSCRYPT_V2) == false) {
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (elKey->StoreKey(auth) == false) {
        elKey->ClearKey();
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (elKey->ActiveKey() == false) {
        elKey->ClearKey();
        LOGE("global security key active failed");
        return -EFAULT;
    }

    if (type == EL1_KEY) {
        userEl1Key_[userId] = elKey;
    } else if (type == EL2_KEY) {
        userEl2Key_[userId] = elKey;
    }
    LOGI("key create success");

    return 0;
}

int KeyManager::RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("enter");
    if (HasElkey(userId, type)) {
        LOGD("The user %{public}u el %{public}u have existed", userId, type);
        return 0;
    }

    std::shared_ptr<BaseKey> elKey = std::make_shared<BaseKey>(dir);
    if (elKey == nullptr) {
        LOGE("No memory for device el1 key");
        return -ENOMEM;
    }

    if (elKey->InitKey(FSCRYPT_V2) == false) {
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (elKey->RestoreKey(auth) == false) {
        elKey->ClearKey();
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (elKey->ActiveKey() == false) {
        elKey->ClearKey();
        LOGE("global security key active failed");
        return -EFAULT;
    }

    if (type == EL1_KEY) {
        userEl1Key_[userId] = elKey;
    } else if (type == EL2_KEY) {
        userEl2Key_[userId] = elKey;
    }
    LOGI("key restore success");

    return 0;
}

bool KeyManager::HasElkey(uint32_t userId, KeyType type)
{
    LOGI("enter");
    if (type == EL1_KEY) {
        if (userEl1Key_.find(userId) != userEl1Key_.end()) {
            LOGD("user el1 key has existed");
            return true;
        }
    } else if (type == EL2_KEY) {
        if (userEl2Key_.find(userId) != userEl2Key_.end()) {
            LOGD("user el2 key has existed");
            return true;
        }
    } else {
        LOGE("key type error");
    }

    return false;
}

int KeyManager::LoadAllUsersEl1Key(void)
{
    LOGI("enter");
    std::vector<FileList> dirInfo;
    ReadDigitDir(USER_EL1_DIR, dirInfo);
    for (auto item : dirInfo) {
        if (RestoreUserKey(item.userId, item.path, NULL_KEY_AUTH, EL1_KEY) != 0) {
            LOGE("user %{public}u el1 key restore error", item.userId);
            return -EFAULT;
        }
    }

    return 0;
}

int KeyManager::InitUserElkeyStorageDir(void)
{
    int ret = MkDir(SERVICE_STORAGE_DAEMON_DIR, 0700);
    if (ret && errno != EEXIST) {
        LOGE("make service storage daemon dir error");
        return ret;
    }

    ret = MkDir(FSCRYPT_EL_DIR, 0700);
    if (ret && errno != EEXIST) {
        LOGE("make service storage daemon dir error");
        return ret;
    }

    ret = MkDir(USER_EL1_DIR, 0700);
    if (ret && errno != EEXIST) {
        LOGE("make el1 storage dir error");
        return ret;
    }
    ret = MkDir(USER_EL2_DIR, 0700);
    if (ret && errno != EEXIST) {
        LOGE("make el2 storage dir error");
        return ret;
    }

    return 0;
}

int KeyManager::InitGlobalUserKeys(void)
{
    LOGI("enter");
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = InitUserElkeyStorageDir();
    if (ret) {
        LOGE("Init user el storage dir failed");
        return ret;
    }

    std::string globalUserEl1Path = USER_EL1_DIR + "/" + std::to_string(GLOBAL_USER_ID);
    if (IsDir(globalUserEl1Path)) {
        ret = RestoreUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Restore el1 failed");
            return ret;
        }
    } else {
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Generate el1 failed");
            return ret;
        }
    }

    std::string globalUserEl2Path = USER_EL2_DIR + "/" + std::to_string(GLOBAL_USER_ID);
    if (!IsDir(globalUserEl2Path)) {
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl2Path, NULL_KEY_AUTH, EL2_KEY);
        if (ret != 0) {
            DoDeleteUserKeys(GLOBAL_USER_ID);
            LOGE("Generate el2 failed");
            return ret;
        }
    }

    ret = LoadAllUsersEl1Key();
    if (ret) {
        LOGE("Load all users el1 failed");
        return ret;
    }
    LOGI("Init global user key success");

    return 0;
}

int KeyManager::GenerateUserKeys(unsigned int user, uint32_t flags)
{
    LOGI("start, user:%{public}u", user);
    std::lock_guard<std::mutex> lock(keyMutex_);
    if ((!IsDir(USER_EL1_DIR)) || (!IsDir(USER_EL2_DIR))) {
        LOGD("El storage dir is not existed, fbe may not be enabled");
        return 0;
    }

    std::string el1Path = USER_EL1_DIR + "/" + std::to_string(user);
    std::string el2Path = USER_EL2_DIR + "/" + std::to_string(user);
    if (IsDir(el1Path) || IsDir(el2Path)) {
            LOGE("user %{public}d el key have existed, create error", user);
            return -EEXIST;
    }
    int ret = GenerateAndInstallUserKey(user, el1Path, NULL_KEY_AUTH, EL1_KEY);
    if (ret) {
        LOGE("user el1 create error");
        return ret;
    }

    ret = GenerateAndInstallUserKey(user, el2Path, NULL_KEY_AUTH, EL2_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el2 create error");
        return ret;
    }
    LOGI("Create user el success");

    return 0;
}

void KeyManager::DoDeleteUserKeys(unsigned int user)
{
    auto it = userEl1Key_.find(user);
    if (it != userEl1Key_.end()) {
        auto elKey = it->second;
        elKey->ClearKey();
        std::string path = elKey->GetDir();
        RmDirRecurse(path);
        userEl1Key_.erase(user);
    }

    it = userEl2Key_.find(user);
    if (it != userEl2Key_.end()) {
        auto elKey = it->second;
        elKey->ClearKey();
        std::string path = elKey->GetDir();
        RmDirRecurse(path);
        userEl2Key_.erase(user);
    }
}

int KeyManager::DeleteUserKeys(unsigned int user)
{
    LOGI("start, user:%{public}d", user);
    std::lock_guard<std::mutex> lock(keyMutex_);
    DoDeleteUserKeys(user);
    LOGI("delete user key success");

    return 0;
}

int KeyManager::UpdateUserAuth(unsigned int user, const std::string &token,
                               const std::string &composePwd)
{
    LOGI("start, user:%{public}d", user);
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (userEl2Key_.find(user) == userEl2Key_.end()) {
        LOGE("Have not found user %{public}u el2 key", user);
        return -ENOENT;
    }

    auto item = userEl2Key_[user];
    UserAuth auth = {
        .token = token,
    };
    if (item->RestoreKey(auth) == false) {
        LOGE("Restoore key error");
        return -EFAULT;
    }
    if (item->StoreKey(auth) == false) {
        LOGE("Store key error");
        return -EFAULT;
    }
    item->keyInfo_.key.Clear();

    return 0;
}

int KeyManager::ActiveUserKey(unsigned int user, const std::string &token,
                              const std::string &secret)
{
    LOGI("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (userEl2Key_.find(user) != userEl2Key_.end()) {
        LOGE("The user %{public}u el2 have been actived", user);
        return 0;
    }
    std::string keyDir = USER_EL2_DIR + "/" + std::to_string(user);
    if (!IsDir(keyDir)) {
        LOGE("Have not found user %{public}u el2", user);
        return -ENOENT;
    }

    std::shared_ptr<BaseKey> elKey = std::make_shared<BaseKey>(keyDir);
    if (elKey->InitKey(FSCRYPT_V2) == false) {
        LOGE("Init el failed");
        return -EFAULT;
    }
    UserAuth auth = {
        .token = token
    };
    if (elKey->RestoreKey(auth) == false) {
        LOGE("Restore el failed");
        return -EFAULT;
    }
    if (elKey->ActiveKey() == false) {
        LOGE("Active user %{public}u key failed", user);
        return -EFAULT;
    }

    userEl2Key_[user] = elKey;
    LOGI("Active user %{public}u el2 success", user);

    return 0;
}

int KeyManager::InActiveUserKey(unsigned int user)
{
    LOGI("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (userEl2Key_.find(user) == userEl2Key_.end()) {
        LOGE("Have not found user %{public}u el2", user);
        return -ENOENT;
    }
    auto elKey = userEl2Key_[user];
    if (elKey->ClearKey() == false) {
            LOGE("Clear user %{public}u key failed", user);
            return -EFAULT;
    }

    userEl2Key_.erase(user);
    LOGI("Inactive user %{public}u el2 success", user);

    return 0;
}

int KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type,
                                     const std::vector<FileList> &vec)
{
    LOGI("start");
    std::string keyPath;
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (type == EL1_KEY) {
        if (userEl1Key_.find(user) == userEl1Key_.end()) {
            LOGE("Have not found user %{public}u el1 key", user);
            return -EINVAL;
        }
        keyPath = userEl1Key_[user]->GetDir();
    } else if (type == EL2_KEY) {
        if (userEl2Key_.find(user) == userEl2Key_.end()) {
            LOGE("Have not found user %{public}u el2 key", user);
            return -EINVAL;
        }
        keyPath = userEl2Key_[user]->GetDir();
    } else {
        LOGD("Not specify el flags, no need to crypt");
        return 0;
    }

    std::string policy = "";
    for (auto item : vec) {
        if (KeyCtrl::LoadAndSetPolicy(keyPath, policy, item.path) == false) {
            LOGE("Set directory el policy error!");
            return -EFAULT;
        }
    }
    LOGI("Set user %{public}u el policy success", user);

    return 0;
}
} // namespace StorageDaemon
} // namespace OHOS

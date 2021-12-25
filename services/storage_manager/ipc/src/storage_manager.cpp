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

#include "ipc/storage_manager.h"
#include "utils/storage_manager_log.h"
#include "system_ability_definition.h"
#include "utils/storage_manager_errno.h"
#include "user/multi_user_manager_service.h"

namespace OHOS {
namespace StorageManager {

REGISTER_SYSTEM_ABILITY_BY_ID(StorageManager, STORAGE_MANAGER_SA_ID , true);

void StorageManager::OnStart()
{
    LOGI("Begin");
    bool res = SystemAbility::Publish(this);
    LOGI("End, res = %{public}d", res);
}

void StorageManager::OnStop()
{
    LOGI("Done");
}

int32_t StorageManager::OnUserCreate(int32_t userId)
{
    LOGI("StorageManager::OnUserCreate start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->OnUserCreate(userId);
    return err;
}

int32_t StorageManager::OnUserDelete(int32_t userId)
{
    LOGI("StorageManger::OnUserDelete start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->OnUserDelete(userId);
    return err;
}
}
}
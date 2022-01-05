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
#include <vector>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

#include "base_key.h"
#include "key_ctrl.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "securec.h"

using namespace testing::ext;

class CryptoKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    OHOS::StorageDaemon::UserAuth emptyUserAuth { "" };
};

std::string toEncryptMnt("/data/lqh/mnt");
std::string toEncryptDir("/data/lqh/mnt/test_crypto");
std::string testKeyPath("/data/test/sys_de");
OHOS::StorageDaemon::BaseKey deKey {testKeyPath};
void CryptoKeyTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void CryptoKeyTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void CryptoKeyTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void CryptoKeyTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: basekey_init
 * @tc.desc: Verify the InitKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_init, TestSize.Level1)
{
    OHOS::StorageDaemon::BaseKey deKey(testKeyPath);

    EXPECT_EQ(true, deKey.InitKey());

    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, deKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.keyDesc.data.get());

    deKey.keyInfo_.key.Clear();
    deKey.keyInfo_.keyDesc.Clear();
}

/**
 * @tc.name: basekey_store
 * @tc.desc: Verify the StoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_store, TestSize.Level1)
{
    std::vector<char> buf {};
    OHOS::ForceRemoveDirectory(testKeyPath);

    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/alias"));
    OHOS::LoadBufferFromFile(testKeyPath + "/alias", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, buf.size());

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/sec_discard"));
    OHOS::LoadBufferFromFile(testKeyPath + "/sec_discard", buf);
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_SECDISC_SIZE, buf.size());

    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/encrypted"));
    OHOS::LoadBufferFromFile(testKeyPath + "/encrypted", buf);
    EXPECT_EQ(80U, buf.size());
}

/**
 * @tc.name: basekey_restore
 * @tc.desc: Verify the RestoreKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_restore, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));

    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_AES_256_XTS_KEY_SIZE, deKey.keyInfo_.key.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.key.data.get());
    EXPECT_EQ(OHOS::StorageDaemon::CRYPTO_KEY_ALIAS_SIZE, deKey.keyInfo_.keyDesc.size);
    EXPECT_NE(nullptr, deKey.keyInfo_.keyDesc.data.get());
}

/**
 * @tc.name: basekey_install
 * @tc.desc: Verify the ActiveKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_install, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(false, deKey.keyInfo_.key.IsEmpty());

    EXPECT_EQ(true, deKey.ActiveKey());
    // raw key should be erase after install to kernel.
    EXPECT_EQ(true, deKey.keyInfo_.key.IsEmpty());
}

/**
 * @tc.name: basekey_clear
 * @tc.desc: Verify the ClearKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_clear, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.RestoreKey(emptyUserAuth));
    EXPECT_EQ(false, deKey.keyInfo_.key.IsEmpty());

    EXPECT_EQ(true, deKey.ClearKey());
    EXPECT_EQ(true, deKey.keyInfo_.key.IsEmpty());
    EXPECT_EQ(true, deKey.keyInfo_.keyDesc.IsEmpty());
}

/**
 * @tc.name: basekey_fscrypt_v2_key
 * @tc.desc: Verify the fscrypt V2 active and clear function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_key, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    EXPECT_EQ(true, deKey.ActiveKeyV2(toEncryptMnt));

    EXPECT_EQ(static_cast<unsigned int>(FSCRYPT_KEY_IDENTIFIER_SIZE), deKey.keyInfo_.keyId.size);
    EXPECT_EQ(true, OHOS::FileExists(testKeyPath + "/kid"));

    EXPECT_EQ(true, deKey.ClearKeyV2(toEncryptMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_set
 * @tc.desc: Verify the fscrypt V2 setpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_set, TestSize.Level1)
{
    EXPECT_EQ(true, deKey.InitKey());
    EXPECT_EQ(true, deKey.StoreKey(emptyUserAuth));
    // the ext4 disk with `mke2fs -O encrypt` mounted for test
    EXPECT_EQ(true, deKey.ActiveKeyV2(toEncryptMnt));

    struct fscrypt_policy_v2 arg = {.version = FSCRYPT_POLICY_V2};
    memcpy_s(arg.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, deKey.keyInfo_.keyId.data.get(), deKey.keyInfo_.keyId.size);
    arg.contents_encryption_mode = OHOS::StorageDaemon::CONTENTS_MODES.at("aes-256-xts");
    arg.filenames_encryption_mode = OHOS::StorageDaemon::FILENAME_MODES.at("aes-256-cts");
    arg.flags = FSCRYPT_POLICY_FLAGS_PAD_32;
    // Default to maximum zero-padding to leak less info about filename lengths.
    OHOS::ForceRemoveDirectory(toEncryptDir);
    OHOS::ForceCreateDirectory(toEncryptDir);
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::SetPolicy(toEncryptDir, arg));

    sleep(10);

    EXPECT_EQ(true, deKey.ClearKeyV2(toEncryptMnt));
    // EXPECT_EQ(true, deKey.ClearKeyV2(toEncryptMnt));
}

/**
 * @tc.name: basekey_fscrypt_v2_policy_get
 * @tc.desc: Verify the fscrypt V2 getpolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(CryptoKeyTest, basekey_fscrypt_v2_policy_get, TestSize.Level1)
{
    struct fscrypt_get_policy_ex_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.policy_size = sizeof(arg.policy);
    std::cout << "policy_size=" << arg.policy_size << std::endl;
    EXPECT_EQ(true, OHOS::StorageDaemon::KeyCtrl::GetPolicy(toEncryptDir, arg));
    EXPECT_EQ(FSCRYPT_POLICY_V2, arg.policy.version);
}

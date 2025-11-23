/**
 * @file UserProfile_test.cpp
 * @brief UserProfile类的单元测试
 * @author FlyKylin Team
 * @date 2025-11-19
 */

#include "core/config/UserProfile.h"
#include <gtest/gtest.h>
#include <QUuid>

using flykylin::core::UserProfile;

class UserProfileTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 单例实例应始终存在
TEST_F(UserProfileTest, SingletonInstanceNotNull) {
    UserProfile* instance = &UserProfile::instance();
    EXPECT_NE(instance, nullptr);
}

// 初始状态：应具有非空的 userId 和 userName，且 UUID 格式有效
TEST_F(UserProfileTest, InitialStateIsValid) {
    const UserProfile& profile = UserProfile::instance();

    QString userId = profile.userId();
    QString userName = profile.userName();

    EXPECT_FALSE(userId.isEmpty());
    EXPECT_FALSE(userName.isEmpty());
    EXPECT_TRUE(profile.isValid());

    QUuid uuid(userId);
    EXPECT_FALSE(uuid.isNull());
}

// setUserName 应更新用户名并保持配置有效
TEST_F(UserProfileTest, SetUserNameUpdatesValueAndStaysValid) {
    UserProfile& profile = UserProfile::instance();

    QString oldName = profile.userName();
    QString newName = oldName + "_Test";

    profile.setUserName(newName);

    EXPECT_EQ(profile.userName(), newName);
    EXPECT_TRUE(profile.isValid());
}

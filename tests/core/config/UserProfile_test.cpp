/**
 * @file UserProfile_test.cpp
 * @brief UserProfile类的单元测试
 * @author FlyKylin Team
 * @date 2025-11-19
 */

#include "core/config/UserProfile.h"
#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QTest>

using namespace FlyKylin::Core::Config;

class UserProfileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前设置
    }
    
    void TearDown() override {
        // 测试后清理
    }
};

// 测试默认构造函数
TEST_F(UserProfileTest, DefaultConstructor) {
    UserProfile profile;
    
    EXPECT_TRUE(profile.uuid().isEmpty());
    EXPECT_TRUE(profile.userName().isEmpty());
    EXPECT_FALSE(profile.isValid());  // 空UUID和用户名，无效
}

// 测试带参数构造函数
TEST_F(UserProfileTest, ParameterizedConstructor) {
    QString testUuid = "a1b2c3d4-e5f6-4012-8901-9abcdef01234";
    QString testName = "TestUser";
    
    UserProfile profile(testUuid, testName);
    
    EXPECT_EQ(profile.uuid(), testUuid);
    EXPECT_EQ(profile.userName(), testName);
    EXPECT_TRUE(profile.isValid());
}

// 测试Getters和Setters
TEST_F(UserProfileTest, GettersAndSetters) {
    UserProfile profile;
    
    profile.setUuid("test-uuid-1234");
    EXPECT_EQ(profile.uuid(), QString("test-uuid-1234"));
    
    profile.setUserName("Alice");
    EXPECT_EQ(profile.userName(), QString("Alice"));
    
    profile.setHostName("Alice-PC");
    EXPECT_EQ(profile.hostName(), QString("Alice-PC"));
    
    profile.setAvatarPath("/avatars/alice.png");
    EXPECT_EQ(profile.avatarPath(), QString("/avatars/alice.png"));
    
    profile.setMacAddress("00:1A:2B:3C:4D:5E");
    EXPECT_EQ(profile.macAddress(), QString("00:1A:2B:3C:4D:5E"));
}

// 测试JSON序列化
TEST_F(UserProfileTest, ToJson) {
    UserProfile profile;
    profile.setUuid("test-uuid");
    profile.setUserName("TestUser");
    profile.setHostName("TestHost");
    profile.setMacAddress("00:11:22:33:44:55");
    profile.setCreatedAt(1700000000);
    profile.setUpdatedAt(1700000000);
    
    QJsonObject json = profile.toJson();
    
    EXPECT_EQ(json["uuid"].toString(), QString("test-uuid"));
    EXPECT_EQ(json["user_name"].toString(), QString("TestUser"));
    EXPECT_EQ(json["host_name"].toString(), QString("TestHost"));
    EXPECT_EQ(json["mac_address"].toString(), QString("00:11:22:33:44:55"));
    EXPECT_EQ(json["created_at"].toInteger(), 1700000000);
    EXPECT_EQ(json["updated_at"].toInteger(), 1700000000);
}

// 测试JSON反序列化
TEST_F(UserProfileTest, FromJson) {
    QJsonObject json;
    json["uuid"] = "test-uuid-5678";
    json["user_name"] = "Bob";
    json["host_name"] = "Bob-Laptop";
    json["mac_address"] = "AA:BB:CC:DD:EE:FF";
    json["created_at"] = 1700000000;
    json["updated_at"] = 1700000100;
    
    UserProfile profile = UserProfile::fromJson(json);
    
    EXPECT_EQ(profile.uuid(), QString("test-uuid-5678"));
    EXPECT_EQ(profile.userName(), QString("Bob"));
    EXPECT_EQ(profile.hostName(), QString("Bob-Laptop"));
    EXPECT_EQ(profile.macAddress(), QString("AA:BB:CC:DD:EE:FF"));
    EXPECT_EQ(profile.createdAt(), 1700000000);
    EXPECT_EQ(profile.updatedAt(), 1700000100);
}

// 测试UUID验证 - 有效格式
TEST_F(UserProfileTest, IsValidUuid_ValidFormat) {
    EXPECT_TRUE(UserProfile::isValidUuid("a1b2c3d4-e5f6-4012-8901-9abcdef01234"));
    EXPECT_TRUE(UserProfile::isValidUuid("00000000-0000-0000-0000-000000000000"));
    EXPECT_TRUE(UserProfile::isValidUuid("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"));
}

// 测试UUID验证 - 无效格式
TEST_F(UserProfileTest, IsValidUuid_InvalidFormat) {
    EXPECT_FALSE(UserProfile::isValidUuid(""));  // 空字符串
    EXPECT_FALSE(UserProfile::isValidUuid("invalid-uuid"));  // 格式错误
    EXPECT_FALSE(UserProfile::isValidUuid("12345678-1234-1234-1234-12345678"));  // 长度不足
    EXPECT_FALSE(UserProfile::isValidUuid("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"));  // 非十六进制
    EXPECT_FALSE(UserProfile::isValidUuid("{a1b2c3d4-e5f6-4012-8901-9abcdef01234}"));  // 带大括号
}

// 测试touch方法（更新时间戳）
TEST_F(UserProfileTest, Touch) {
    UserProfile profile;
    qint64 initialTime = 1700000000;
    profile.setUpdatedAt(initialTime);
    
    EXPECT_EQ(profile.updatedAt(), initialTime);
    
    // 等待一小段时间
    QTest::qWait(10);
    
    // 调用touch，应该更新updatedAt
    profile.touch();
    
    EXPECT_GT(profile.updatedAt(), initialTime);
}

// 测试isValid方法
TEST_F(UserProfileTest, IsValid) {
    UserProfile profile;
    
    // UUID和用户名都为空 - 无效
    EXPECT_FALSE(profile.isValid());
    
    // 只有UUID - 无效
    profile.setUuid("test-uuid");
    EXPECT_FALSE(profile.isValid());
    
    // 只有用户名 - 无效
    profile.setUuid("");
    profile.setUserName("TestUser");
    EXPECT_FALSE(profile.isValid());
    
    // 都有 - 有效
    profile.setUuid("test-uuid");
    profile.setUserName("TestUser");
    EXPECT_TRUE(profile.isValid());
}

// 测试序列化和反序列化的往返
TEST_F(UserProfileTest, SerializationRoundTrip) {
    UserProfile original;
    original.setUuid("roundtrip-uuid");
    original.setUserName("RoundTripUser");
    original.setHostName("RT-Host");
    original.setAvatarPath("/avatars/rt.png");
    original.setMacAddress("12:34:56:78:9A:BC");
    original.setCreatedAt(1700000000);
    original.setUpdatedAt(1700000100);
    
    // 序列化
    QJsonObject json = original.toJson();
    
    // 反序列化
    UserProfile deserialized = UserProfile::fromJson(json);
    
    // 验证所有字段
    EXPECT_EQ(deserialized.uuid(), original.uuid());
    EXPECT_EQ(deserialized.userName(), original.userName());
    EXPECT_EQ(deserialized.hostName(), original.hostName());
    EXPECT_EQ(deserialized.avatarPath(), original.avatarPath());
    EXPECT_EQ(deserialized.macAddress(), original.macAddress());
    EXPECT_EQ(deserialized.createdAt(), original.createdAt());
    EXPECT_EQ(deserialized.updatedAt(), original.updatedAt());
}

// 测试边界条件 - 空JSON
TEST_F(UserProfileTest, FromJson_EmptyJson) {
    QJsonObject emptyJson;
    
    UserProfile profile = UserProfile::fromJson(emptyJson);
    
    // 应该得到空的profile
    EXPECT_TRUE(profile.uuid().isEmpty());
    EXPECT_TRUE(profile.userName().isEmpty());
    EXPECT_FALSE(profile.isValid());
}

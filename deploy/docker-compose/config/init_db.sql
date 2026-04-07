-- 创建数据库
CREATE DATABASE IF NOT EXISTS chat_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE chat_db;

-- 用户基础表
CREATE TABLE IF NOT EXISTS `users` (
    `user_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '用户唯一ID',
    `username` VARCHAR(64) NOT NULL COMMENT '登录账号',
    `password_hash` VARCHAR(128) NOT NULL COMMENT '密码哈希',
    `nickname` VARCHAR(64) NOT NULL DEFAULT '' COMMENT '用户昵称',
    `avatar_url` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '头像链接',
    `status` TINYINT NOT NULL DEFAULT 0 COMMENT '0:离线 1:在线',
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '注册时间',
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`user_id`),
    UNIQUE KEY `idx_username` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=10000 DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- 好友关系表
CREATE TABLE IF NOT EXISTS `user_friends` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `user_id` BIGINT UNSIGNED NOT NULL COMMENT '我的ID',
    `friend_id` BIGINT UNSIGNED NOT NULL COMMENT '好友ID',
    `status` TINYINT NOT NULL DEFAULT 1 COMMENT '1:正常 2:拉黑',
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `idx_user_friend` (`user_id`, `friend_id`),
    KEY `idx_friend_id` (`friend_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='好友关系表';

-- 消息索引表 (写扩散模型)
CREATE TABLE IF NOT EXISTS `message_index` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `receiver_id` BIGINT UNSIGNED NOT NULL COMMENT '接收者(信箱所有人)',
    `seq_id` VARCHAR(64) NOT NULL COMMENT '严格递增的消息序号(排序/断点拉取)',
    `msg_id` VARCHAR(64) NOT NULL COMMENT '全局唯一消息ID(对应MongoDB主键)',
    `session_type` TINYINT NOT NULL DEFAULT 1 COMMENT '1:单聊 2:群聊',
    `sender_id` BIGINT UNSIGNED NOT NULL COMMENT '发送者',
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `idx_receiver_seq` (`receiver_id`, `seq_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息收件箱索引';

-- 预置测试数据
INSERT IGNORE INTO `users` (user_id, username, password_hash, nickname) VALUES 
(1001, 'test_user', '123456_hashed', 'Alice'),
(2002, 'test_friend', 'abcdef_hashed', 'Bob');

INSERT IGNORE INTO `user_friends` (user_id, friend_id) VALUES 
(1001, 2002),
(2002, 1001);

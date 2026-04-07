# 数据库设计文档

本项目采用业界主流的 **混合存储架构**（MySQL + MongoDB + Redis），以满足 IM 系统中“消息量大”、“读取频率高”、“弱关系性”以及“强社交属性”等复杂业务场景。

## 1. 核心架构设计

### 1.1 为什么采用 MySQL + MongoDB 的混合模型？
- **关系数据（MySQL）**：用户的基本信息、好友关系、群组列表等，这些数据通常关系较强、修改频繁，需要支持事务，适合 MySQL。
- **消息索引（MySQL）**：利用 MySQL 的 B+ 树或自增机制，为用户的信箱生成严格递增的序列号 (`seq_id`)，这是实现“断点续传”和“离线消息拉取”的核心。
- **消息本体（MongoDB）**：单条聊天消息（尤其是富文本、媒体 URL 等 JSON 数据）可能比较庞大。MongoDB 的文档存储天然契合 JSON，并且具备出色的写入吞吐量和内置分片（Sharding）能力，适合存储海量弱结构化的流水数据。

### 1.2 会话状态（Redis）
- 存放用户的会话 `Token`、在线状态（在线/离线）。
- 存储活跃用户的设备信息（用于离线消息决定推送至哪个平台：iOS/Android）。

---

## 2. 表结构设计

### 2.1 MySQL 表结构 (库名：`chat_db`)

#### a) 用户基础表 (`users`)
存放用户的基本登录信息。
| 字段名          | 类型             | 约束/备注                     |
| --------------- | ---------------- | ----------------------------- |
| `user_id`       | BIGINT UNSIGNED  | PK, 自增起始 10000            |
| `username`      | VARCHAR(64)      | 登录账号，UNIQUE KEY          |
| `password_hash` | VARCHAR(128)     | 密码哈希值 (Bcrypt 等)        |
| `nickname`      | VARCHAR(64)      | 昵称                          |
| `avatar_url`    | VARCHAR(255)     | 头像链接                      |
| `status`        | TINYINT          | 0:离线 1:在线                 |

#### b) 用户关系表 (`user_friends`)
存放用户之间的好友关注关系（双向则存两行）。
| 字段名       | 类型             | 约束/备注                                    |
| ------------ | ---------------- | -------------------------------------------- |
| `id`         | BIGINT UNSIGNED  | PK, 自增                                     |
| `user_id`    | BIGINT UNSIGNED  | 拥有者                                       |
| `friend_id`  | BIGINT UNSIGNED  | 目标好友                                     |
| `status`     | TINYINT          | 1:正常 2:拉黑                                |
| `created_at` | TIMESTAMP        | 关注时间                                     |

*(联合唯一索引：`idx_user_friend(user_id, friend_id)`)*

#### c) 消息索引表 (`message_index`)
负责实现**写扩散**（将消息通知投递到收件人的收件箱）或群聊的**读扩散**。
| 字段名         | 类型             | 约束/备注                                           |
| -------------- | ---------------- | --------------------------------------------------- |
| `id`           | BIGINT UNSIGNED  | PK, 自增                                            |
| `receiver_id`  | BIGINT UNSIGNED  | 收件人 (或群 ID)                                    |
| `seq_id`       | VARCHAR(64)      | **核心**: 严格递增序号 (如基于时间戳/雪花/MySQL自增)|
| `msg_id`       | VARCHAR(64)      | 对应 MongoDB 的 `_id` 或 `msg_id`                   |
| `session_type` | TINYINT          | 1:单聊 2:群聊                                       |
| `sender_id`    | BIGINT UNSIGNED  | 发件人                                              |

*(联合唯一索引：`idx_receiver_seq(receiver_id, seq_id)`，用于范围扫描拉取增量 `WHERE receiver_id=? AND seq_id > ?`)*

---

### 2.2 MongoDB 集合设计 (库名：`chat_db`)

#### a) 消息本体集合 (`messages`)
存放聊天内容的具体数据，按 `msg_id` 分片。
```json
{
  "_id": ObjectId("..."),
  "msg_id": "MSG-UUID-12345678",        // 全局唯一 UUID (客户端生成)
  "sender_id": 1001,
  "receiver_id": 2002,
  "session_type": 1,                    // 1:单聊 2:群聊
  "msg_type": 1,                        // 1:文本 2:图片 3:语音
  "content": "Hello! I am fine.",       // 对于图片可能是 {"url": "http://..", "size": 1024} 的 JSON 字符串
  "send_time": 1770000000000,           // 客户端发送时间戳
  "server_time": 1770000001000          // 服务器接收时间戳
}
```

---

### 2.3 Redis 键值设计

#### a) 用户会话 Token (`String`)
- **Key**: `user:token:{user_id}`
- **Value**: `TK_1001_1_1775545334_3033`
- **TTL**: 7天（验证时自动续期）

#### b) 活跃设备缓存 (`Hash` 或 `String`)
- **Key**: `user:device:{user_id}`
- **Value**: `{"platform": 1, "token": "apns_device_token_xxxx"}`
- **用途**: `Push` 服务下发苹果/谷歌推送前，通过 Redis 高速判断用户当前的设备类型与通道 Token。

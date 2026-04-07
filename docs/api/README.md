# IM 系统 API 接口文档

本文档描述客户端通过网关（Gateway）与微服务交互的核心接口。
> **注：** 本项目采用 gRPC+Protobuf 作为底层微服务通信。Gateway 作为门面，既支持短连接 HTTP (RESTful JSON)，也支持长连接 TCP (Protobuf/自定义协议)。

## 1. HTTP 短连接 API (网关层映射 User 服务)

### 1.1 用户注册 `POST /api/user/register`
- **说明**：创建新用户账号。
- **请求 Body (JSON)**:
  ```json
  {
      "username": "test_user",
      "password": "password123",
      "nickname": "TestUser"
  }
  ```
- **响应 Body (JSON)**:
  ```json
  {
      "code": 0,
      "msg": "SUCCESS",
      "data": {
          "user_id": 3003
      }
  }
  ```

### 1.2 用户登录 `POST /api/user/login`
- **说明**：验证密码，签发 JWT/Token 会话。
- **请求 Body (JSON)**:
  ```json
  {
      "username": "test_user",
      "password": "password123",
      "client_type": 1, 
      "device_id": "uuid-xxxx-yyyy"
  }
  ```
- **响应 Body (JSON)**:
  ```json
  {
      "code": 0,
      "msg": "SUCCESS",
      "data": {
          "token": "TK_1001_1_1775545334_3033",
          "user_info": {
              "user_id": 1001,
              "nickname": "TestUser",
              "avatar_url": "http://example.com/avatar.jpg"
          }
      }
  }
  ```

### 1.3 拉取好友列表 `GET /api/user/friends`
- **Header**: `Authorization: Bearer <Token>`
- **Query Params**: `page=1`, `page_size=20`
- **响应 Body (JSON)**:
  ```json
  {
      "code": 0,
      "msg": "SUCCESS",
      "data": {
          "total": 2,
          "list": [
              {"user_id": 2001, "nickname": "Alice", "status": 1},
              {"user_id": 2002, "nickname": "Bob", "status": 0}
          ]
      }
  }
  ```

---

## 2. TCP 长连接协议 (网关层映射 Logic 服务)

客户端连接到 Gateway TCP 端口（如 9000）后，采用自定义封包：`[4字节长度][2字节命令字][Protobuf序列化数据]`。

### 2.1 客户端发消息 (CMD: `SEND_MSG_REQ`)
- 对应 Protobuf `SendMsgReq`
- **字段**:
  - `msg_id`: 客户端生成的 UUID
  - `receiver_id`: 接收方 UserID / GroupID
  - `session_type`: 1(单聊) / 2(群聊)
  - `msg_type`: 1(文本) / 2(图片)
  - `content`: 字节流(文本或 JSON 结构)

### 2.2 服务端推送消息 (CMD: `PUSH_MSG_NOTIFY`)
- 对应 Protobuf `PushMsgNotify`
- 当有离线或在线新消息时，网关主动下发给客户端。包含服务端的 `server_time` 与严格递增的 `seq_id`。

### 2.3 消息回执 (CMD: `MSG_ACK_REQ`)
- 对应 Protobuf `MsgAckReq`
- 客户端收到 `PUSH_MSG_NOTIFY` 后必须回复，更新自己的已读或已收 `seq_id`，防止下次登录重复下发。

### 2.4 拉取漫游离线消息 (CMD: `PULL_OFFLINE_REQ`)
- 对应 Protobuf `PullOfflineMsgReq`
- 断线重连后，客户端带着本地最新一条消息的 `seq_id` 来请求，服务端返回大于该 `seq_id` 的增量消息列表。

---

## 3. 推送厂商自定义 Payload (APNs / FCM)

当用户处于离线且 TCP 断开状态，消息流经 `PushManager`，会组装以下 JSON 下发给 Apple/Google：

### Apple APNs (iOS)
```json
{
  "aps": {
    "alert": {
      "title": "New Message",
      "body": "Hello World!"
    },
    "badge": 1,
    "sound": "default"
  },
  "custom_data": {
    "msg_id": "mock_123"
  }
}
```

### Google FCM (Android)
```json
{
  "to": "device_token_xxxx",
  "notification": {
    "title": "New Message",
    "body": "Hello World!"
  },
  "data": {
    "custom": {
      "msg_id": "mock_123"
    }
  }
}
```

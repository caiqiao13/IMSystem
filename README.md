# IMSystem (chat-system)

## 你能用它做什么
- **Gateway**：对外 TCP 长连接 + HTTP API 入口（示例端口：9000/8080）
- **Logic**：聊天业务处理（敏感词过滤、路由到 Message/Push 等）
- **User**：注册/登录/Token/好友关系（依赖 MySQL/Redis）
- **Message**：消息存储（MongoDB 存本体、MySQL 存索引）
- **Push**：离线推送（订阅 RabbitMQ 事件，调用 APNs/FCM）
- **Admin**：管理端（如有）

## 目录结构
```text
common/        公共组件（日志、配置、工具等）
proto/         Protobuf 定义
gateway/       网关服务（TCP/HTTP）
logic/         逻辑服务（gRPC）
user/          用户服务（gRPC）
message/       消息服务（gRPC）
push/          推送服务
deploy/        docker-compose 与 k8s 部署
docs/          架构/API/数据库/部署文档
```

## 快速开始（推荐：Docker Compose）
前置：安装 Docker 与 Docker Compose。

在项目根目录执行：
```bash
cd deploy/docker-compose
docker-compose up -d --build
```

### 验证
- **查看容器状态**

```bash
docker-compose ps
```

- **查看网关日志**

```bash
docker-compose logs -f gateway
```

- **测试登录接口（示例）**

```bash
curl -X POST http://localhost:8080/api/user/login \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"test_user\",\"password\":\"password123\",\"client_type\":1,\"device_id\":\"dev-1\"}"
```

## 本地编译（CMake）
### 方式 A：使用 vcpkg（Windows 推荐）
安装并配置 vcpkg 后，安装依赖：

```bash
vcpkg install spdlog jsoncpp asio
```

然后在配置时传入 vcpkg toolchain（示例）：

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### 方式 B：允许 CMake 自动拉取依赖（需要网络）
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCHAT_SYSTEM_FETCH_DEPS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

产物默认在 `build/bin/`。

## 配置说明
每个服务都有自己的配置文件：
- `gateway/config/gateway_config.yaml`
- `logic/config/logic_config.yaml`
- `user/config/user_config.yaml`
- `message/config/message_config.yaml`
- `push/config/push_config.yaml`

服务启动时会读取各自 YAML（例如 `server.*`、`log.*`、`dependencies.*`）。

## 文档
- `docs/architecture/README.md`：架构设计
- `docs/api/README.md`：API 协议
- `docs/database/README.md`：数据库设计
- `docs/deployment/README.md`：部署与运维
# IM 部署与运维指南

本项目支持**本地一键联调（Docker Compose）**与**生产环境集群（Kubernetes）**两种主流的云原生部署模式。我们强烈建议采用基于 Docker 的部署方式，以避免不同操作系统和依赖库版本不一致导致的编译错误。

## 1. 环境依赖要求

- 操作系统：Linux (Ubuntu 22.04+ / CentOS 8+) 或 macOS
- 软件环境：
  - Docker (>= 20.10.x)
  - Docker Compose (>= 2.x)
  - （可选）Kubernetes 集群 (Minikube / k3s / EKS / ACK)

---

## 2. 一键本地开发/联调环境 (Docker Compose)

利用 Docker 的**多阶段构建 (Multi-stage build)**，您无需在宿主机上安装 `g++`、`CMake`、`spdlog` 等繁琐的开发工具链。一条命令即可完成 C++ 项目的**编译 -> 镜像打包 -> 容器拉起**。

### 2.1 启动服务

进入项目根目录：
```bash
cd chat-system/deploy/docker-compose

# 构建镜像并在后台拉起所有中间件与微服务
docker-compose up -d --build
```

**启动流程概览**：
1. Docker 会自动读取 `deploy/docker/Dockerfile`。
2. 内部启动 Ubuntu 环境拉取 `cmake`、`gRPC`、`Redis` 等库。
3. 编译五个微服务二进制 (`gateway`, `logic`, `user`, `message`, `push`)。
4. 按 `depends_on` 顺序启动 `MySQL`, `Redis`, `MongoDB`, `RabbitMQ`。
5. MySQL 首次启动会自动执行 `init_db.sql`，建立用户表和消息索引。
6. 微服务全部上线。

### 2.2 验证运行状态

```bash
# 查看所有容器的运行状态
docker-compose ps

# 查看网关服务的日志（观察是否有报错或断线）
docker-compose logs -f gateway

# 查看逻辑服务的日志（观察敏感词拦截等）
docker-compose logs -f logic
```

### 2.3 测试连接

本地可用 `curl` 测试短连接 API（例如发给 Gateway 的 8080 端口，路由至 User 服务）：

```bash
curl -X POST http://localhost:8080/api/user/login \
     -H "Content-Type: application/json" \
     -d '{"username": "test_user", "password": "password123", "client_type": 1}'
```

*(预期返回 200 OK 及签发的 JWT Token 凭证)*

### 2.4 停止与清理

```bash
# 停止并移除容器（保留数据卷）
docker-compose down

# 如果需要彻底清空 MySQL / Redis 数据，加上 -v
docker-compose down -v
```

---

## 3. 生产环境集群部署 (Kubernetes)

为了应对百万级并发长连接，线上需通过 K8s 的 Deployment 和 HPA（水平伸缩）实现 Gateway 与 Logic 的弹性部署。

### 3.1 准备镜像

首先需要将打包好的镜像推送到您的私有镜像仓库：
```bash
docker build -t registry.example.com/chat/gateway:latest -f deploy/docker/Dockerfile .
docker push registry.example.com/chat/gateway:latest
# 同理为 logic, user, message, push 打 Tag 并推送
```

### 3.2 部署资源清单

进入 K8s 部署目录，依次 Apply 资源清单：
```bash
cd chat-system/deploy/k8s

# 1. 创建命名空间
kubectl apply -f namespace.yaml

# 2. 部署核心网关层 (对公网暴露 TCP 和 HTTP 端口)
kubectl apply -f gateway/deployment.yaml
# Gateway 会创建 LoadBalancer 或 NodePort，接收外部海量终端的长连接。

# 3. 部署核心逻辑层 (内部集群 RPC)
kubectl apply -f logic/deployment.yaml
# Logic 会创建 ClusterIP，专供 Gateway 进行高速 gRPC 通信调用。

# 4. （可选）部署用户、消息存储、离线推送
kubectl apply -f user/deployment.yaml
kubectl apply -f message/deployment.yaml
kubectl apply -f push/deployment.yaml
```

### 3.3 监控与扩容

```bash
# 查看 Pod 状态
kubectl get pods -n chat-system

# 当流量洪峰来临时，可手动或配置 HPA 自动横向扩容网关
kubectl scale deployment chat-gateway --replicas=10 -n chat-system
```

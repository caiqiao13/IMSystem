# ==========================================
# 阶段 1: 编译构建环境 (Builder)
# ==========================================
FROM ubuntu:22.04 AS builder

# 替换为国内镜像源并安装编译依赖
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake gdb \
    libssl-dev zlib1g-dev \
    libmysqlclient-dev \
    libhiredis-dev \
    librabbitmq-dev \
    protobuf-compiler libprotobuf-dev \
    libgrpc++-dev protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /build

# 将整个项目拷贝进容器
COPY . .

# 执行 CMake 编译
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# ==========================================
# 阶段 2: 运行时环境 (Runtime)
# ==========================================
FROM ubuntu:22.04

# 安装运行时必需的动态库
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libssl3 zlib1g \
    libmysqlclient21 \
    libhiredis0.14 \
    librabbitmq4 \
    libprotobuf23 \
    libgrpc++1.43 \
    ca-certificates tzdata \
    && rm -rf /var/lib/apt/lists/*

# 设置时区
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

WORKDIR /app

# 从 builder 阶段拷贝编译好的二进制文件
COPY --from=builder /build/build/bin/gateway_server /app/
COPY --from=builder /build/build/bin/logic_service /app/
COPY --from=builder /build/build/bin/user_service /app/
COPY --from=builder /build/build/bin/message_service /app/
COPY --from=builder /build/build/bin/push_service /app/

# 设置默认启动的入口为 gateway，实际在 docker-compose 中会覆盖 entrypoint
CMD ["/app/gateway_server"]

#!/usr/bin/env bash
set -euo pipefail

# 脚本所在目录（BuildTools）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# 默认安装到 /usr/local/ChatSDK，可用环境变量覆盖（便于在无 root 环境下测试）
INSTALL_ROOT="${INSTALL_ROOT:-/usr/local/ChatSDK}"

echo "==> 1/3 编译静态库 libChatSDK.a ..."
mkdir -p "${SCRIPT_DIR}/build"
cd "${SCRIPT_DIR}/build"
cmake "${SCRIPT_DIR}" >/dev/null
cmake --build . -j"$(nproc)"

# 安装阶段：若当前用户对目标父目录没有写权限且不是 root，则用 sudo
SUDO=""
if [ "$(id -u)" -ne 0 ] && [ ! -w "$(dirname "${INSTALL_ROOT}")" ]; then
    SUDO="sudo"
fi

echo "==> 2/3 安装到 ${INSTALL_ROOT} ..."
${SUDO} mkdir -p "${INSTALL_ROOT}/lib" "${INSTALL_ROOT}/include" "${INSTALL_ROOT}/Third_Party/Httplib"

# 安装静态库
${SUDO} cp -f "${SCRIPT_DIR}/libChatSDK.a" "${INSTALL_ROOT}/lib/"

# 安装 Include 目录全部内容（含 Util 子目录）
${SUDO} cp -rf "${PROJECT_DIR}/ChatSDK/Include/." "${INSTALL_ROOT}/include/"

# Common.h 相对引用了 ../Third_Party/Httplib/httplib.h，需保持相对路径可用
${SUDO} cp -f "${PROJECT_DIR}/ChatSDK/Third_Party/Httplib/httplib.h" "${INSTALL_ROOT}/Third_Party/Httplib/"

echo "==> 3/3 安装完成：${INSTALL_ROOT}"
echo ""
echo "使用示例："
echo "  g++ main.cpp -std=c++17 -DCPPHTTPLIB_OPENSSL_SUPPORT \\"
echo "      -I${INSTALL_ROOT}/include -L${INSTALL_ROOT}/lib -lChatSDK \\"
echo "      -ljsoncpp -lfmt -lspdlog -lsqlite3 -lssl -lcrypto -lpthread -o MyApp"

#!/usr/bin/env bash
set -euo pipefail

# 默认卸载 /usr/local/ChatSDK，可用环境变量覆盖（便于在无 root 环境下测试）
INSTALL_ROOT="${INSTALL_ROOT:-/usr/local/ChatSDK}"

SUDO=""
if [ "$(id -u)" -ne 0 ] && [ -e "${INSTALL_ROOT}" ] && [ ! -w "$(dirname "${INSTALL_ROOT}")" ]; then
    SUDO="sudo"
fi

if [ -e "${INSTALL_ROOT}" ]; then
    ${SUDO} rm -rf "${INSTALL_ROOT}"
    echo "卸载完成：${INSTALL_ROOT} 已删除"
else
    echo "未发现安装目录：${INSTALL_ROOT}，无需卸载"
fi

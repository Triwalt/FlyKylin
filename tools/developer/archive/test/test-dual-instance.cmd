@echo off
REM ========================================
REM 双实例测试脚本 - UDP节点发现
REM ========================================

cd /d %~dp0

echo.
echo ========================================
echo FlyKylin 双实例测试
echo ========================================
echo.
echo 功能: 测试UDP节点发现
echo 预期: 两个实例应相互发现并显示在列表中
echo.
echo 操作步骤:
echo 1. 两个窗口将自动打开
echo 2. 等待3-5秒让节点相互发现
echo 3. 检查两个窗口的"在线用户"列表
echo 4. 应该看到对方节点出现在列表中
echo 5. 测试搜索框过滤功能
echo.
echo ========================================
echo.

REM 启动第一个实例
echo [1/2] 启动第一个实例...
start "FlyKylin Instance 1" build\bin\Release\FlyKylin.exe

REM 等待2秒
timeout /t 2 /nobreak >nul

REM 启动第二个实例
echo [2/2] 启动第二个实例...
start "FlyKylin Instance 2" build\bin\Release\FlyKylin.exe

echo.
echo ✓ 两个实例已启动
echo.
echo 验证清单:
echo [ ] 两个窗口都显示"服务启动成功"
echo [ ] 等待5秒后，两个窗口都显示"在线：1人"
echo [ ] 可以看到对方的用户信息（主机名/IP）
echo [ ] 搜索框可以过滤用户列表
echo [ ] 双击用户显示功能提示（US-002待实现）
echo.
echo 按任意键关闭所有实例...
pause >nul

REM 结束所有FlyKylin进程
taskkill /F /IM FlyKylin.exe >nul 2>&1

echo.
echo ✓ 测试完成
echo.

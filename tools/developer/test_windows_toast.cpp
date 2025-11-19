/**
 * Windows Toast通知权限测试工具
 * 编译: cl /EHsc /std:c++17 test_windows_toast.cpp /link windowsapp.lib
 * 运行: test_windows_toast.exe
 */

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>

using namespace winrt;
using namespace Windows::UI::Notifications;
using namespace Windows::Data::Xml::Dom;

bool testToastNotification() {
    try {
        // Initialize WinRT
        init_apartment();
        
        // Create Toast XML
        std::wstring xmlStr = 
            L"<toast>"
            L"  <visual>"
            L"    <binding template='ToastGeneric'>"
            L"      <text>FlyKylin Test</text>"
            L"      <text>This is a test notification. If you see this message, Toast permissions are working.</text>"
            L"    </binding>"
            L"  </visual>"
            L"</toast>";
        
        // Load XML
        XmlDocument toastXml;
        toastXml.LoadXml(xmlStr);
        
        // Create Toast notification
        ToastNotification toast(toastXml);
        
        // Show notification
        ToastNotificationManager::CreateToastNotifier(L"FlyKylin").Show(toast);
        
        std::cout << "[SUCCESS] Toast notification sent!" << std::endl;
        std::cout << "Please check Windows notification center." << std::endl;
        std::cout << "\nIf you don't see the notification, possible reasons:" << std::endl;
        std::cout << "1. Windows notifications are disabled (Settings > System > Notifications)" << std::endl;
        std::cout << "2. Focus Assist is enabled" << std::endl;
        std::cout << "3. Need to create app shortcut in Start Menu" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Toast notification failed: " << e.what() << std::endl;
        return false;
    }
}

#else
bool testToastNotification() {
    std::cout << "[INFO] Toast notifications require Windows 10 or later" << std::endl;
    return false;
}
#endif

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  FlyKylin - Windows Toast Notification Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
#ifdef _WIN32
    std::cout << "Attempting to send Toast notification..." << std::endl;
    std::cout << "Note: Toast notifications require Windows 10 or later" << std::endl;
    std::cout << std::endl;
    
    bool success = testToastNotification();
    
    if (success) {
        std::cout << "\nPress any key to exit..." << std::endl;
        std::cin.get();
        return 0;
    } else {
        std::cout << "\nTest failed, recommend using QSystemTrayIcon as fallback" << std::endl;
        return 1;
    }
#else
    std::cout << "This test tool is only supported on Windows platform" << std::endl;
    return 1;
#endif
}

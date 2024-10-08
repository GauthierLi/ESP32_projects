#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h> 
#include <libserialport.h>  // libserialport 库头文件
#include <iomanip> 

using namespace std;

// 定义一个结构体用于存储内存和显存信息
struct SystemUsage {
    long totalMemory;
    long freeMemory;
    double ramUsagePercent;
    int totalGpuMemory;
    int usedGpuMemory;
    double gpuUsagePercent;
};

// 获取系统内存使用情况
SystemUsage getMemoryUsage() {
    SystemUsage usage{};
    string line;
    ifstream memInfo("/proc/meminfo");

    if (memInfo.is_open()) {
        long totalMemory = 0, freeMemory = 0;

        while (getline(memInfo, line)) {
            if (line.find("MemTotal:") == 0) {
                totalMemory = stol(line.substr(line.find_first_of("0123456789")));
            } else if (line.find("MemAvailable:") == 0) {
                freeMemory = stol(line.substr(line.find_first_of("0123456789")));
                break;
            }
        }
        
        memInfo.close();
        double usagePercent = ((totalMemory - freeMemory) / (double)totalMemory) * 100;

        usage.totalMemory = totalMemory;
        usage.freeMemory = freeMemory;
        usage.ramUsagePercent = usagePercent;
    } else {
        cerr << "Unable to read memory info" << endl;
    }

    return usage;
}

// 获取显卡内存使用情况
void getGpuMemoryUsage(SystemUsage &usage) {
    FILE* fp = popen("nvidia-smi --query-gpu=memory.used,memory.total --format=csv,noheader,nounits", "r");
    if (fp == nullptr) {
        cerr << "Unable to fetch GPU memory usage" << endl;
        return;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        int used, total;
        sscanf(buffer, "%d, %d", &used, &total);
        double usagePercent = (double)used / total * 100.0;

        usage.usedGpuMemory = used;
        usage.totalGpuMemory = total;
        usage.gpuUsagePercent = usagePercent;
    } else {
        cerr << "Unable to fetch GPU memory usage" << endl;
    }

    pclose(fp);
}

int main() {
    // libserialport 库初始化
    struct sp_port *port;
    sp_return result = sp_get_port_by_name("/dev/ttyUSB0", &port);
    if (result != SP_OK) {
        cerr << "Unable to find port" << endl;
        return 1;
    }

    result = sp_open(port, SP_MODE_WRITE);
    if (result != SP_OK) {
        cerr << "Unable to open port" << endl;
        return 1;
    }

    // 设置波特率为 115200
    sp_set_baudrate(port, 115200);

    while (true) {
        // 获取内存和显存信息
        SystemUsage usage = getMemoryUsage();
        getGpuMemoryUsage(usage);

        // 组装信息字符串
        stringstream message;
        message << fixed << setprecision(2);
        message << "Ubuntu Monitor\n";
        message << "RAM Memory: " << usage.totalMemory / (1024 * 1024) << " GB\n";
        message << "RAM Free: " << usage.freeMemory / (1024 * 1024) << " GB\n";
        message << "RAM Usage: " << usage.ramUsagePercent << "%\n";
        message << "GPU Memory: " << usage.totalGpuMemory / 1024.0 << " GB\n";
        message << "GPU Used: " << usage.usedGpuMemory / 1024.0 << " GB\n";
        message << "GPU Usage: " << usage.gpuUsagePercent << "%\n";
        message << "END";

        // 发送信息到 ESP32
        string messageStr = message.str();
        sp_nonblocking_write(port, messageStr.c_str(), messageStr.size());
        sp_drain(port);  // 清空数据缓冲区

        sleep(5);  // 延迟一秒  
    }

    sp_close(port);
    sp_free_port(port);
    return 0;
}

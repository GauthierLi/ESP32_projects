#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // 创建 TFT_eSPI 对象
String receivedData = "";   // 用于存储接收的数据

void setup() {
  Serial.begin(115200);  // 初始化串口通信，波特率为 115200
  tft.init();            // 初始化 TFT 显示屏
  tft.setRotation(3);    // 设置屏幕方向
  tft.fillScreen(TFT_BLACK);  // 填充背景为黑色
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // 设置文本颜色为白色，背景为黑色
  tft.setTextSize(2);  // 设置文本大小

  // 初始提示信息
  tft.setCursor(10, 10);
  tft.println("Waiting for data...");
}

void loop() {
  // 读取串口中的所有数据
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    receivedData += receivedChar;

    // 当检测到 "END" 标志符时处理数据
    if (receivedData.indexOf("END") != -1) {
      // 找到 "END" 的位置
      int endIndex = receivedData.indexOf("END");
      
      // 提取从开始到 "END" 之前的字符串
      String dataToProcess = receivedData.substring(0, endIndex);

      // 清除上一帧内容
      tft.fillScreen(TFT_BLACK);

      // 解析数据并显示
      int yPosition = 20;  // 起始Y坐标，初始行位置
      tft.drawLine(1, yPosition - 7, 239, yPosition - 7, TFT_YELLOW);
      tft.drawLine(1, yPosition + 7 + tft.textWidth("A") , 239, yPosition + 7 + tft.textWidth("A"), TFT_YELLOW);
      tft.drawFastVLine(1, yPosition - 7, 232, TFT_YELLOW);
      tft.drawFastVLine(239, yPosition - 7, 232, TFT_YELLOW);
      tft.drawFastHLine(1, 239, 232, TFT_YELLOW);
      int lineSpacing = 30;  // 每行之间的间距

      int valueXPos = 230;  // 数值右对齐位置

      // 将数据按行分割处理
      int lineStart = 0;
      while (lineStart < dataToProcess.length()) {
        int lineEnd = dataToProcess.indexOf('\n', lineStart);
        if (lineEnd == -1) {
          lineEnd = dataToProcess.length();
        }

        String line = dataToProcess.substring(lineStart, lineEnd);

        // 显示左侧前缀部分
        int colonIndex = line.indexOf(':');
        if (colonIndex != -1) {
          String prefix = line.substring(0, colonIndex + 1);
          String value = line.substring(colonIndex + 1);
          value.trim();  // 去除数值前的空格，直接在原字符串上修改

          // 设置左侧前缀位置
          tft.setCursor(10, yPosition);
          tft.setTextColor(TFT_WHITE, TFT_BLACK);  // 设置前缀部分为白色
          tft.print(prefix);

          // 解析数值并决定颜色
          float usageValue = value.toFloat();
          if (usageValue < 50.0) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);  // 低于50%显示绿色
          } else if (usageValue < 80.0) {
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);  // 介于50%-80%显示黄色
          } else {
            tft.setTextColor(TFT_RED, TFT_BLACK);  // 高于80%显示红色
          }

          // 设置右侧数值的位置
          int valueWidth = tft.textWidth(value);
          tft.setCursor(valueXPos - valueWidth, yPosition);
          tft.print(value);
        } else {
          // 如果没有冒号，直接显示整行内容
          tft.setCursor(10, yPosition);
          tft.setTextColor(TFT_WHITE, TFT_BLACK);  // 默认显示为白色
          tft.print(line);
        }

        // 更新行起始位置
        lineStart = lineEnd + 1;
        // 更新Y坐标位置，增加行间距
        yPosition += lineSpacing;
      }

      // 删除已处理的数据，包括 "END"
      receivedData = receivedData.substring(endIndex + 3);
    }
    // delay(100);  // 延迟以避免频繁刷新
  }

}

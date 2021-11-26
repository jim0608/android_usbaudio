# android_usbaudio
基于libusb，实现无驱动获取USBAudio

## 期望实现的功能：
- 通过libusb获取USBAudio数据

## 部分摄像头无法获取音频问题解决思路：
- 无法获取音频的原因：当前传过去的采样率在设备当前选择的interface中不存在！
- 之前没声音是因为：我们打开的设备声卡通道它当前自己有一个固定的采样率，主机这边传过去的采样率和设备当前的端点的采样率不匹配，就会导致拿不到音频数据
    - 如：设备当前采样率为32000，主机传过去的采样率为48000，就会导致拿去不到音频数据
- 罗技4K无声音：当前选择bAlternateSetting的interface中采样率为48000HZ，然而主机传递传递过去想要设置的采样率在当前interface下不存在；
- RAPOO4K无声音是因为传递过去的采样率在当前的interface下不存在

## 获取设备采样率、通道数、bit
- 获取当前libusb_interface_descriptor下的extra
- 解析extra，可获取到当前interface的通道数，采样率等；

## 问题记录
### 记录在开发这个项目时，遇到的问题
1. 使用libusb_control_transfer，报错LIBUSB_ERROR_PIPE()
- 原因是当前传过去的参数错误，如：bmRequestType：（LIBUSB_ENDPOINT_OUT |LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT），wIndex:（EndpointAddress），并且部分摄像头必须要求设置采样率
2. 如何设置采样率
- UAC协议中，采样率数据长度为3，给libusb_control_transfer长度为3的data中设置
```c++
data[0] = (rate & 0xff);
data[1] = (rate >> 8);
data[2] = (rate >> 16);
```

## 感谢以下相关资料，对我的帮助
- [Universal Serial Bus Device Class Definition for Audio Devices 1.0](https://www.usb.org/sites/default/files/audio10.pdf)
- 
- [UAC规范（USB音频](http://www.usbzh.com/article/forum-1.html),这是对UAC相关协议部分中文资料

## 该项目是我个人第一次写c/c++相关代码，有不合理的望大家可以指正，谢谢


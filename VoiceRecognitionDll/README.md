# VoiceRecognitionDll

这个项目生成给 LabVIEW 调用的 .NET Framework DLL。

## 识别词和响应

- 你好 -> 你好啊
- 下午好 -> 下午好呀
- 中午好 -> 中午好呀
- 晚上好 -> 晚上好呀
- 分析 -> 好的
- 音乐 -> 正在为您打开网易云

## LabVIEW 调用顺序

1. 放置 `.NET Constructor Node`，选择 `VoiceRecognitionDll.dll` 里的 `VoiceRecognitionDll.VoiceRecognizer`。
2. 接 `.NET Invoke Node` 调用 `Init`。
3. 接 `.NET Invoke Node` 调用 `Start`。
4. 停止按钮里调用 `Stop`。
5. 程序退出前可调用 `Dispose`。

不要把 `Start` 和 `Stop` 直接连续执行，否则会刚开始识别就停止。

## 重新编译

如果改了 `VoiceRecognizer.cs`，在 PowerShell 里运行：

```powershell
.\build.ps1
```

生成的 DLL 在：

```text
bin\Release\VoiceRecognitionDll.dll
```

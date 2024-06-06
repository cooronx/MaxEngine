# MaxEngine
如果想使用vscode+clangd进行编码，**需要配置%LocalAppData%\clangd\config.yaml**  
添加以下代码(检查你的windows kit路径)
```
CompileFlags:
  Add: [
    "-IC:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\winrt",
  ]
```
需要自定义的路径
1. vcpkg.cmake
2. vcvarsall.bat

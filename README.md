# 使用C语言实现一个兼容多平台多GUI的chess程序
实现思路：
底层逻辑使用C语言实现，尽量做到与不同平台的渲染和显示解耦，通过提供接口与外部进行交互。
初步设想需要兼容windows,linux,imx6ull嵌入式Linux开发板（大概率使用LVGL或者QT），有可能的话兼容苹果手机，微信小程序（可以不做，感觉兼容比较困难，小程序使用的是javascript）等
底层尽量使用C语言，上层可以根据平台适配不同语言
问题：
1. 项目的组织结构应该怎么做？有哪些文件夹，文件夹有哪些文件？
2. 为了适配不同平台是否需要做一个平台适配层？GUI层通过调用平台适配层提供的统一接口？
3. 项目构建工具：因为对make比较熟悉，初步使用makefile进行项目构建，后续项目升级时再考虑使用cmake
4. 优化相关（先不考虑）：1. 棋子和棋盘图片的加载，是否需要使用图片加载库？2.是否支持解析常见棋谱文件
进度：
1. 目前已完成C语言核心走棋逻辑的代码，通过终端命令行控制验证算法逻辑基本正确；目前能够在ubuntu中通过GTK库进行运行；
2. 目前已经重构项目架构，整体分为port层，controller层和核心层core三个层次。实现棋盘大小支持随意拉伸，棋盘，棋子等元素可以通过素材自定义，只是目前素材采用的是PNG图片。
## 环境准备
### ubuntu 20.04
安装gtk库
执行：
```bash
sudo apt update
sudo apt install -y libgtk-3-dev pkg-config
```
验证gtk是否安装成功
```bash
pkg-config --cflags --libs gtk+-3.0
```
输出类似：
```bash
dku@dku:~/.local/share/Trash/files/chess_lgorithm$ pkg-config --cflags --libs gtk+-3.0
-pthread -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
```
## 获取项目
```bash
git clone git@github.com:dkyou/chess.git
```

## 编译和运行
GTK (默认):
```bash
make           # same as: make TARGET=gtk
make run
```

CLI stub:
```bash
make clean && make TARGET=cli && make run
```


## 项目组织结构
### 项目组织结构第一版
整体分为port层，controller层和核心层core三个层次
```bash
chess_project/
├─ include/                        # 全局公共头
│  ├─ common.h
│  ├─ platform.h
│  └─ chess.h                      # ← 对外API
│
├─ src/
│  ├─ core/                        # 纯C核心
│  │  ├─ include/chess/core.h      # 核心共享少量类型/工具
│  │  ├─ chess.c                   # ← 你的原始核心逻辑已移入
│  │  ├─ move.c                    # 预留（当前 stub，后续可拆分）
│  │  └─ game.c                    # 预留（当前 stub）
│  │
│  ├─ platform/                    # 平台适配层基础实现
│  │  ├─ include/pal/event_bus.h
│  │  ├─ platform_base.c           # 时间/文件/日志的通用版本
│  │  └─ event_bus.c               # 简单事件总线
│  │
│  └─ ports/
│     ├─ linux_gtk/                # Linux+GTK 端口
│     │  ├─ include/gtk/ui.h       # ← 你的 gtk_ui.h
│     │  ├─ gtk_ui.c               # ← 你的 gtk_ui.c（include 已适配）
│     │  ├─ gtk_platform.c         # GdkPixbuf 图像实现（PAL示例）
│     │  └─ main_gtk.c             # 入口（原 main.c 拆出）
│     └─ terminal/                 # 终端端口（最小运行stub）
│        ├─ main_cli.c
│        └─ terminal_platform.c
│
├─ tools/make/
│  ├─ cli.mk
│  └─ gtk.mk
├─ Makefile
└─ README.md

```
### 项目组织结构第二版
进一步抽象


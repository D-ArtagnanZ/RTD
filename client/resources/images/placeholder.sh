#!/bin/bash

# 确保convert命令可用
if ! command -v convert &> /dev/null; then
    echo "需要安装ImageMagick。请运行: apt-get install -y imagemagick"
    exit 1
fi

# 创建所有必要的图标
convert -size 32x32 xc:lightblue -fill navy -gravity center -draw "text 0,0 'EQP'" equipment.png
convert -size 32x32 xc:lightgreen -fill darkgreen -gravity center -draw "text 0,0 'LOT'" lot.png
convert -size 32x32 xc:lightpink -fill darkred -gravity center -draw "text 0,0 'DIS'" dispatch.png
convert -size 32x32 xc:lightyellow -fill orange -gravity center -draw "text 0,0 'RTD+'" logo.png
convert -size 32x32 xc:white -fill blue -gravity center -draw "text 0,0 'R'" refresh.png
convert -size 32x32 xc:white -fill purple -gravity center -draw "text 0,0 'S'" search.png
convert -size 32x32 xc:white -fill green -gravity center -draw "text 0,0 '+'" add.png
convert -size 32x32 xc:white -fill gray -gravity center -draw "text 0,0 '*'" settings.png

echo "所有图标文件已创建!"

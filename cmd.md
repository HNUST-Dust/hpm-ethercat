# hpm-ethercat 构建命令(在 hpm-ethercat 下执行)

# 首次构建 / 修改 prj.conf、overlay 后(干净重建)
west build -p -b hpm6e00evk -d build

# 日常改代码后(增量编译,更快)
west build 

# 烧录(板载 FT2232 + openocd)
west flash


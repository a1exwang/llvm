# LLVM V9Cpu Backend

## 设计文档
[设计文档](alex-machine-docs/index.md)

## 成员参与信息
- 王奥丞
    - llvm工具链
    - nodejs版模拟器中调试器部分
    - 通过elf生成v9 excuatble和网页版模拟器的调试符号文件的脚本

### TODOs
- 近期目标
    - 编译器
        - ~~变长参数~~
        - ~~结构体作为函数参数~~
        - ~~结构体作为函数返回值~~
        - ~~函数指针~~
        - ~~取地址~~
        - ~~32位常量Load~~
        - ~~a > b ? a : b~~
        - ~~无符号算数~~
        - ~~小范围case~~
        - ~~大范围case~~
        - \*\*浮点数
    - 编译器已知bug
        - ~~调用函数没有保存所有called saved regsiters,
            而且保存的寄存器有不必要的~~
        - ~~调用非函数指针函数时, 偏移不能超过16位~~
        - ~~不同长度整数转换~~
        - ~~无符号整数被当成有符号(still exists?)~~
        - long long乘法, 目前只能高位是0
        - ~~8位/1位整数符号扩展~~
        - AlexInstrLowering.cpp有一些未翻译的LLVM IR
        - AlexInstrInfo::InsertBranch not implemented
        - ~~Function Epologue~~
    - 编译器目标代码生成模块
        - 检查每条指令的二进制代码是否正确
        - ~~opcode > 7位的指令暂时生成错误~~
    - AlexMachine缺少的指令
        - ~~ORi $ra, $rb, imm~~
        - ~~LBU/LHU $ra, $rb, imm, 无符号(填0)扩展~~
        - ~~将一个寄存器最低位零扩展到32位~~
        - MULHS long long的乘法, 可以软件实现?
    - 不能处理的LLVM IR
        - ~~anyext (貌似可以用clang -O2消除掉这条指令)~~
    - ~~elf转换器, 提取elf中的符号信息~~
        - ~~symbol address~~
        - ~~line number~~
        - frame info
- 长期目标
    - 编译器
        - 要变成伪指令的指令
            - ~~lbit, lbits~~
            - ~~lbs~~
            - ~~lhs~~
            - ~~sext_inreg~~
                - ~~所有符号扩展指令用shl/sar实现~~
            - ~~lw/sw~~
            - ~~select~~
        - 系统指令
            - 获得pc
            - ~~设置时钟中断~~
    - 汇编器: 完成v0.1
        - ~~llvm-mc~~
    - 内联汇编: 完成v0.1
        - 测试
            - ~~constraints: r, =r~~
            - constraints: c
        - 可能的问题:
            - ~~立即数长度~~
    - 连接器
        - ~~全局变量~~
        - ~~函数指针~~
        - ~~局部跳转~~
        - ~~DWARF调试信息生成(源码级调试)~~
        - ~~DWARF未测试~~
        - ~~32bit data~~


# LLVM V9Cpu Backend

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
        - 无符号整数被当成有符号(still exists?)
        - long long乘法, 目前只能高位是0
        - ~~8位/1位整数符号扩展~~
        - ~~AlexInstrLowering.cpp有一些未翻译的LLVM IR~~
        - AlexInstrInfo::InsertBranch not implemented
        - Function Epilogue
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
    - ~~elf转v9可执行格式~~
        - 完成, 项目地址 https://github.com/a1exwang/alex2v9
        - 使用方法ruby main.rb a.out result.v9
    - elf转换器, 提取elf中的符号信息
- 长期目标
    - 编译器
        - 简化AlexMachine的指令集使其更像V9Cpu, 最终可以通过V9Cpu的模拟器运行
        - 要变成伪指令的指令
            - ~~lbit, lbits~~
            - ~~lbs~~
            - ~~lhs~~
            - ~~sext_inreg~~
                - 所有符号扩展指令用shl/sar实现
            - ~~mov $ra, $rb~~
                - push $ra
                - pop $rb
            - ~~add $ra, $rb, $rc~~(所有算数逻辑指令)
                - push RA
                - push RB
                - mov RA, $rb
                - mov RB, $rc
                - add
                - mov $ra, RA
                - pop RA
                - pop RB
            - addiu $ra, $rb, imm
            - ~~lw $ra, SP, imm~~
                - push RA
                - mov RA, $ra
                - ll imm
                - mov $ra, RA
                - pop RA
            - sw SP, imm, $ra
                - push RA
                - mov RA, $ra
                - sl imm
                - pop RA
            - 普通function call
                - ent 建立堆栈
                - jsr 调用函数
                - lev 返回
            - 函数指针调用: call $ra
                - push RA (used to store return address)
                - push $ra (used to store function address)
                - push RA (used to backup RA)
                - leag x
                - load RA, SP\[0\]
                - pop $ra
                - LEV
            - load全局变量
                - li32 RA, Offset
                - mov RB, Base
                - add
                - lx 0
            - load/store frame index
                -应该是4bytes对齐
            - ~~le, ge, ble, bge~~, 以及他们的无符号指令
            - ~~j~~
                - jmp
            - ~~select~~
                - r = trueReg * condition + falseReg * !condition
        - 16bit操作数改为24bit
        - 减少通用寄存器的个数
        - ~~去掉fp~~
        - 解决编译警告
        - ~~用xem跑通汇编写的helloworld~~
        - ~~用xem跑通c写的helloworld~~
            - pushi翻译的有bug
            - 32bit imm hi 8 bit
        - osN.c
    - 汇编器: 完成v0.1, 未测试
    - 内联汇编: 完成v0.1, 未测试
        - 可能的问题:
            - 立即数长度
    - 连接器
        - ~~全局变量~~
        - ~~函数指针~~
        - ~~局部跳转~~
        - ~~DWARF调试信息生成(源码级调试)~~
        - DWARF未测试


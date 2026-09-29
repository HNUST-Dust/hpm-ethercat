#该代码将Ethercat的相关代码嵌入zephyr

关于Ethercat的相关的代码，包括ESC外设，flash模拟eeprom的相关函数是以
原生sdk调用出来的，相关函数以用户层的代码的形式加入的.其他外设仍是靠设备树获取

##环境配置
1.需要zephyr的工程环境
2.需要sdk-glue内的板级支持包(该项目以 hpm6e00evk为工程模板)
3.需要hpm-sdk

##工程架构
总工程
|-- CMakeLists.txt
|-- src   （源文件）
    |--app_layer (Ethercat的应用层)
    |--bsp
         |--ecat_board_cfg.h(板级的宏定义，若更换板子重点更改这里)
         |--ecat_zephyr_gpio(兼容zephyr的gpio函数)
         |--ecat_zephyr_port(兼容zephyr的中断函数)
    |--port      (Ethercat的硬件层)
         |--hpm_ecat_e2p_emulation.c(模拟eeprom的flash读写的函数调用)
         |--hpm_ecat_hw             (ESC 外设初始化，涉及驱动，中断函数)
         |--hpm_ecat_phy            (phy的相关IO)
    |--ssc       (Ethercat的协议栈 --涉及到Ethercat的状态和数据处理)
    |--main.c    (进程入口)

##快速入门

```c
	/* Zephyr port: board_init() equivalent, ESC clock/pins + PHY reset hold */
	ecat_board_init();
	/* Zephyr port: IRQ_CONNECT for ESC/GPTMR before hardware init enables them */
	ecat_irq_init();
     //使能ESC外设和相关中断
     MainInit(); /* SSC Initilize the stack */
     //Ethercat协议栈启动，配置对象字典
     pAPPL_CoeWriteInd = CoeWriteInd; /* get notified when master writes objects via SDO */

   /*在 ssc/digital_ioObjects.h注册  ssc在解析邮箱数据时会被索引到这些对象
    CoeWriteInd里面执行的就是一个主站SOD对0x2000写之后的读取.pAPPL_CoeWriteInd
    发生在主站通过 SDO 写(下载)任意对象、且写入成功之后被调用*/
    MainLoop();
    //Ethercat解析主循环,从站状态变化，数据解析都发生在这
```

关于过程数据SM2主站可以依靠FMMU找到并进行数据写入
src/app_layer/app_ethercat.c

```c
void APPL_Application(void)
{
    static UINT32 lastSm2 = 0;
    UINT32 rawSm2;
    /* 直读SM2原始缓冲区(主站写入的输出数据,地址来自栈读回的SM配置) */
    HW_EscReadDWord(rawSm2, nEscAddrOutputData);
    if (rawSm2 != lastSm2) {
        lastSm2 = rawSm2;
        printk("SM2: 0x%08x\n", rawSm2);
    }
    InputCounter0x6000 = rawSm2; /* 回环:主站写什么就回什么,SOEM侧可验证 */
    APPL_SetLed(rawSm2);
}
//示例，主站写SM2后会进入中断然后进入该函数(SM同步中断默认开启了)
```

编译下载见 cmd.md

##注意事项

本代码仅在6e00evk上得到验证，移植代码需
注意ESC外设的IP核是否适配

##代码移植

src下4个文件夹直接打包进zephyr工程即可
不同板级要注意src/bsp/ecat_board_cfg.h的宏定义

##协议栈

根据不同业务需求，重点更改对象字典(例如更改过程数据的长度)

##版本迭代

version  --1.0

version  --1.1 过程数据量拓展到32字节长度
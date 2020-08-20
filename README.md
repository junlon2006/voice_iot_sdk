# hb_m_hybird_sdk
蜂鸟M 离在线混合模式跨平台移植方案SDK  
当前为Linux POSIX APIs版本  
目录结构：  
app   -->  应用层源码，main入口函数  
hal   -->  系统跨平台相关APIs  
sdk   -->  提供在线、UART通信核心能力  
utils -->  工程依赖组件  

移植步骤：  
step1、UART初始化，提供UART收发能力  [该部分需要根据平台自行研发]  
源码参考 app/app.c --> UartInitialize  
Tips：UART接收到的数据，需要异步送入通信协议栈CommProtocolReceiveUartData  
请参考app/src/uni_uart.c --> _create_worker_thread，特别强调请异步处理  
即，UART recv到的数据先入缓存队列，在另外一个线程中将缓存数据送入协议栈  

step2、协议栈初始化  
源码参考 app/app.c --> CommProtocolRegisterHooks、CommProtocolInit  
逻辑不需要做任何变动，直接照着写就可以  

step3、在线识别初始化  
参考源码 app/app.c --> ChnlInit  

step4、跨平台POSIX APIs  
hal/inc/porting.h  
hal/src/porting.c  
请根据您的平台，实现该部分所有APIs的移植，请不要修改任何APIs申明格式  

移植完毕。  
关于配网相关的流程细节，后续再做说明  

Linux x86环境demo编译与运行  
step1、在工程根目录hb_m_hybird_sdk执行build.sh，进行编译  
step2、编译完成后，将蜂鸟M和Linux pc将UART连接好，通过dmesg命令查找到设备名   
step3、执行可执行程序sudo build/app/src/APP /dev/ttyUSB0  

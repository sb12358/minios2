minios目前已经完成的功能：
bootsector
进入保护模式
内存分配模块
简单的线程调度模块
信号量
时钟
延时调用DPC和定时调用TPC
统一的设备驱动模型
标准输入输出设备驱动
内存及字符串相关的标准C库函数


附件codes.zip的目录结构如下：
codes
|-relocate        连接程序的源代码，将bootsector和minios连接成一个可启动的磁盘镜像
|-bootsector      bootsector的源代码
|-minios          minios的源代码
|-bin             所有的目标都在此目录中。其中minios.vhd就是可启动的磁盘镜像

如何启动minios：
你必须安装Microsoft的Virtual PC 2007
你可以在微软的官方网站下载他的安装程序，程序大小约30M
http://download.microsoft.com/download/8/5/6/856bfc39-fa48-4315-a2b3-e6697a54ca88/32%20BIT/setup.exe
安装完成后就可以双击codes/bin/vm.vmc运行minios了

如何编译minios:
编译minios共需要三种编译器。
codes/bootsector/bootsector.asm必须用nasm进行编译，将编译的结果命名为bootsector并且拷贝到codes/bin
codes/minios/platform/platform.asm必须用masm32进行编译，编译的结果在codes/minios/platform/platform.obj
其余的代码都用vc6编译即可，vc6的工程在codes/minios/minios.dsw
如果你手边没有nasm和masm32，不要紧，因为这两个文件一般不需要改动，直接用我编译好的目标文件就可以了

双击minios.dsw打开vc6，点击菜单Project->Project Setting->Debug,修改Executable for debug session一栏
将Virtual PC.exe的完整路径填入。如果你安装在默认的路径下，就不需要修改它。
然后直接Ctrl-F5运行就可以编译并且运行了。


vc工程的具体配置和链接原理

为什么使用vc：
原因很简单，因为我不大熟悉gcc，要控制gcc做出教复杂的编译链接操作不大懂
我其实还是希望用gcc的，如果谁能帮我写一个gcc的makefile我很感谢。
代码中有些地方我偷懒了，gcc可能遍不过，不会很多，改一下就可以了

vc工程是在dll的工程的基础上配置的
1、将所有相关的文件加到工程中来。
2、由于对于debug版本的代码生成，vc会加入不少调试代码，不好控制，所以删除Win32 Debug的配置
3、由于默认的Release配置中，会加入Intrinsic Functions的优化，他会用vc libc中的函数代替你写的标准C语言库函数。因此必须自定义优化方案。project setting->C++->Optimizations选customize并且勾上除了Assume No Aliasing, Generate Intrinsic Functions, Favor Small Code, Full Optimization外的优化选项。
4、在project setting->C++->Preprocessor->Additional include directories中加入include这个目录，并且勾上Ignore standard include paths
5、project setting->Link中，output file name改成../bin/minios.dll。勾上Ignore all default libraries和Generate mapfile, object/libraty modules中的内容清空
6、project setting->Link->Debug中mapfile name改成../bin/minios.map,project setting->Link->Output中Entry-point symbol改成main
7、project setting->post-build step中添加一行"../bin/relocate.exe" ../bin/minios.dll ../bin/bootsector ../bin/minios.vhd
8、project setting->Debug中Executable for debug session改成C:\Program Files\Microsoft Virtual PC\Virtual PC.exe，working directory改成../bin，Program arguments改成-singlepc -pc vm -launch
如果我没有忘记什么的话，应该就这些了。这样你的vc就可以编译minios的原代码了。编译的结果在../bin/minios.dll

为什么使用dll的工程呢？
因为windows的dll中有一个relocation的段，他列出了该dll文件如果要重定位的话所有需要修改的地址偏移。假设dll默认的加载位置是0x10000000，而在minios中我希望把它定位在0x400000则只需要把重定位表的每一项所指向的地址减去(0x10000000- 0x400000)就可以了。这也是relocate.exe这个程序的主要工作。

至于具体pe文件的结构以及重定向表的结构，网上有很多，我手边暂时没有资料，可以参看relocate.exe的原代码


minios的引导过程和内存布局

首先，pc机的bios程序会将bootsector加载到0x7C00, 此时段寄存器的值我也不大清楚，但是不要紧，自己重新设一遍吧。把ds, es, ss都设成cs一样的值，把sp放在0x8000的位置上，这样我们就有了512字节的堆栈了。

然后，bootsector将minios读出放在从0x400000开始的内存空间上。随后bootsector简单的设置了GDT后直接进入保护模式并且将控制权交给minios的entrypoint。从0x100000到0x3fffff是内核堆，内核所需要的动态内存都可以从此堆上使用keMalloc和keFree分配。内存最低的4K字节被用来存放中断向量指针，就像纯DOS那样。从0x4000开始到0x8000存放了PCI总线配置数据块。从0x10000到0x1ffff的64K字节用来作为IDE的DMA内存块，其他的低端内存暂时还没有分配，可能会作为文件系统的缓存。

main函数先重新配置了8259中断控制器，8253时钟控制器，设置了IDT GDT，初始化时钟，内核堆和任务子系统后，建立了第一个任务main，入口是keEntryMain

keEntryMain首先打开中断并且加载console和keboard的驱动，然后建立DPC任务，kdebug任务以及一个测试任务

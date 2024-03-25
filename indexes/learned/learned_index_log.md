# Learned_Index_rewrite log
- zmindex.hpp
  - [class ZMIndexInterface]:[S]

  /*
  * the zm-index structure:
  */
- mlindex.hpp
  - [class mlindexInterface]:[S]

  24 year:[try to build] [build success]
- ifindex.hpp
  - [class ifindexInterface][s]
- flood.hpp
  - [class floodInterface][S]
- lisa.hpp
  -  [class LISAInterface]:原作者并没有使用这个LISA类
- lisa2.hpp
  - [class LISA2Interface][S]
- rsmi.hpp
  - [class RSMIInterface] :
    - 29号，编写接口，编译成功，开始测试，提示[进程kill,exit]
    - 1号，添加调试代码，希望暴露更多提示信息，追踪路径:：
      - rsmi.hpp:@build():插入检查点->
      _rsmi.build()->插入检查点->
      RSMI.cpp:@build->插入检查点->
      net->train_model()->
      locate:ModelTools.h::mse_loss[后来发现不是loss函数，是传入的forward]
      ->定位到@forward::x=relu(fc1->forward(x))
      try:
        1.更换其他的激活函数，Fail
        2.输出x的张量形状和内容，看不出有问题,Fail
        3.替换为自己写的custom_relu,还是没有错误信息输出，Fail

      挫败感很大，继续调试也不行了，因为涉及到libtorch的函数实现了，而libtorch里面并没有具体的函数实现，而是打包好的二进制库，也没办法再添加调试信息了
    - 2号：猜测可能是接口写的有问题，继续检查接口函数。查阅pytorch api文档，并检索是否有其他人遇到相似的问题
      - 猜测可能是依赖库的问题，rsmi总共有两个依赖库:libtorch和boost,重点关注libtorch
        - 更换为最新版本:compiler error
        - 更换为前一版本:仍然不行
    - 3号:打算复现一下github的RSMI库:
      - pytorch:1.4.0 linux版本，boost1_7_9
      - compiler [S],测试运行：生成数据需要导入tensorflow,在虚拟机尝试了好久也不能成功运行data_generator.py
      - try:在本地的anaconda+pycharm运行成功经过修改的data_generator.py，生成了uniform.csv
      - 测试：[浮点数异常，Fail]
      - 检查：发现main函数根本没有运行
      - 不知道该怎么继续调试了
    - 4号:查看RSMI的issue,发现可能是makefile里没有加-ltorch_cpu,还有可能是pytorch版本问题
      - /usr/bin/ld:cannot find lxxx！！！！重新下载的pytorch一不小心下成window版本了，浪费了好长时间
      - 还是不行，仍然提示[浮点数异常]，Exp.cpp的main也没有运行
      - 查看别人fork的修改，尝试，还是不行啊

      - 再尝试用1.6.0 version的pytorch+MLIB的rsmi实现，以为大概率不行
      - ：！！！！！！！！！！！！！！！！！！！!!!!!!!!!!!!!!!!!
      - 我去，竟然输出了relu end,应该是成功了啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊
      - 去除之前的checkpoint信息
    - try to merge RSMI to other indices
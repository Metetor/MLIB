# rewrite code log record
[get_index]:[rtree,S] [add rstar support:todo]
[IndexManager] 整体框架使用IndexManager Class进行管理,IndexSet使用各种IndexInterface class代替，build_index使用接口类的函数取代
    [member:index_type,fpath,N,mode,Points]
    [parse_args():解析参数，初始化member]
    [IndexInterface初始化]:"../indexes/indexInterface.h" [RTreeInterface编译成功，需要继续测试]
    [@get_index实现，返回接口类对象]
    [实现@handle_queries，功能实现查询]
[delete IndexSet、build_index]


[merge bench_rsmi.cpp to bench.h]

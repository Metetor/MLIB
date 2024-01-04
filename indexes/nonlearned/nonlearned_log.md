# nonLearned_Index_rewrite log
rtree.hpp:
    [class rtreeInterface:public indexInterface]:逐步替代掉原有的Rtree类
        [@build(Points)]:construct rtree index(originate from RTree(Points))
        [error:]/mnt/hgfs/MLIB/bench/bench.h:24:17: error: invalid new-expression of abstract class type ‘bench::index::RtreeInterface<std::array<double, 2>, 2, 128>’
        24 | index = new bench::index::RtreeInterface<KEY_TYPE, dim>;
        [checkout:build函数实现错误，传入参数不匹配]
        [S]
        [根据IndexInterface class的修改进行了一些变动]
        []

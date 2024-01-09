# nonLearned_Index_rewrite log
- rtree.hpp:

  - [class rtreeInterface:public indexInterface]:

        [@build(Points)]:construct rtree index(originate from RTree(Points))
        [error:]/mnt/hgfs/MLIB/bench/bench.h:24:17: error: invalid new-expression of abstract class type ‘bench::index::RtreeInterface<std::array<double, 2>, 2, 128>’
        24 | index = new bench::index::RtreeInterface<KEY_TYPE, dim>;
        [checkout:build函数实现错误，传入参数不匹配]
        [S]
        [根据IndexInterface class的修改进行了一些变动]
        [range_query函数实现参考原先rtree的实现] [S]
        [try:delete rtree class] [catch:] [S]

  - [class rstarInterface:public indexInterface]:

        [todo like rtree]
        [S]
- kdtree.hpp:
  - [class KDTreeInterface]

    [尝试是否能够去掉或者优化struct KDTreeVectorOfVectorsAdaptor,写得有点太丑陋了]

- ann.hpp:
  - [class ANNBoxDecompositionTree疑似没有使用]
  - [class ANNKDTreeInterface]

        [注释掉看看,就是没有使用，compile success],后期再研究一下作者到底是想干什么
        [重写 C(ompiler)S(uccess)]
        [T(est)S(uccess)]
        [delete ANNKDTree class]
- geos.hpp
  - [class QDTreeInterface]

      [start rewriten]:
      [s]

- uniform_grid.hpp:
  - [class UGInterface]
    [S]

- equal_depth_grid.hpp:
  - [class EDGInterface]
    
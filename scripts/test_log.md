- [rtree and rstar]

        rstar的build，query均比rtree差，后续需要看一下原因在哪
- [real data]:
  - learned index测试失败，debug:
        - mli:error:Points must be increasing by x
        - 搜索:readme里说当初benchmark本地测试就有，可能是代码本身的问题；具体位置:pgm:piece_linear_model:add_point(){if (points_in_hull > 0 && x <= last_x)
            throw std::logic_error("Points must be increasing by x.");}
            调用这个函数的是make_segmentation（）函数
        查看mli的一维学习型索引正是pgm
        first,we add sort to projections,to test the order problem.ohh fuccck,forget to add print log code,
        猜测是不是映射值有相同的呢，不满足映射原则了,查看project函数和论文的project
        add code:
        unique()+erase()去重，nytaxi上面build success,range query success；但是knn query好像死了 fuxxxxxk
        接下来测试fs dataset,两个query都给成功了，这不对啊

        ---/ 先重新处理一下数据集，把nytaxi的重复值删除掉

        现在确定应该就是project策略有问题，或者说pgm存在缺陷，不支持重复值




        查看论文locate project函数:




        先修改一下pgm试试
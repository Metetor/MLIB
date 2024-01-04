# INDEX-Interface.log.md
[class IndexInterface:
    取代base_index,实现计时和build、query的虚函数
    [**rename** some typename]
    [add virtual range query and knn query declare:TODO]
    [add build_time,point_time,range_time,knn_time,and point_cnt,range_cnt,knn_cnt]
    [class impl error:invalid new-expression of abstract class type]:原因大概率是派生类有未实现的成员，这里是因为虚函数

]
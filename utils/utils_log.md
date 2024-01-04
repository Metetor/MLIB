# rewrite utils code log record
type.hpp:
[1.typedef无效别名(无用),]
    [do]
    [Delete vec_of_float_vec_t,knn_t,knnf_t]
    [Delete vec_of_double_vec_t][referred func @read_data @to_points,直接替换为二维double数组]
    [Delete pointf_t,point2_t-point12_t,pointf2_t-pointf12_t,knn2_t-knn12_t,boxf_t,vec_of_pointf_t]
    [done]


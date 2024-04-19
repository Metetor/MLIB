# import matplotlib.pyplot as plt
# import numpy as np
#
# # 生成大于50个随机点
# num_points = 100
# np.random.seed(8)
# x = np.random.rand(num_points)
# y = np.random.rand(num_points)
#
# # 绘制随机点
# plt.scatter(x, y, color='blue', alpha=0.7)
#
# # 定义查询矩形
# x_min = np.random.uniform(0.1, 0.4)
# y_min = np.random.uniform(0.1, 0.4)
# x_max = np.random.uniform(0.6, 0.9)
# y_max = np.random.uniform(0.6, 0.9)
#
# # 计算矩形的选择度
# rect_area = (x_max - x_min) * (y_max - y_min)
# total_area = 1.0  # 假设总面积为1
# density = num_points / total_area
#
# # 绘制查询矩形
# plt.gca().add_patch(plt.Rectangle((x_min, y_min), x_max-x_min, y_max-y_min, linewidth=1, edgecolor='r', facecolor='none'))
#
# # 标记矩形内的点为红色
# for i in range(num_points):
#     if x_min <= x[i] <= x_max and y_min <= y[i] <= y_max:
#         plt.scatter(x[i], y[i], color='red')
#
# # 添加标题和坐标轴标题
# plt.title('Spatial Range Query')
# plt.xlabel('X Axis')
# plt.ylabel('Y Axis')
#
# # 隐藏横纵坐标的数值
# plt.xticks([])
# plt.yticks([])
#
# plt.savefig("range_query.pdf")
# plt.show()

# import matplotlib.pyplot as plt
# import numpy as np
# from sklearn.neighbors import KNeighborsClassifier
#
# # 生成大于50个随机点
# num_points = 100
# np.random.seed(0)
# X = np.random.rand(num_points,2)
#
# # 为所有点分配类别为0
# y = np.zeros(num_points)
#
# # 创建KNN分类器
# k = 7  # k值大于5
# knn = KNeighborsClassifier(n_neighbors=k)
# knn.fit(X, y)
#
# # 生成查询点
# query_point = np.array([[0.5, 0.5]])
#
# # 进行KNN查询
# distances, indices = knn.kneighbors(query_point)
#
# # 绘制随机点
# plt.scatter(X[:, 0], X[:, 1], color='blue')
#
# # 标记查询点
# plt.scatter(query_point[:, 0], query_point[:, 1], marker='x', color='red')
#
# # 连接查询点和其邻居
# for neighbor_index in indices[0]:
#     plt.plot([query_point[0, 0], X[neighbor_index, 0]], [query_point[0, 1], X[neighbor_index, 1]], '--', color='purple')
#
# plt.title('KNN Query')
# plt.xlabel('X Axis')
# plt.ylabel('Y Axis')
# plt.xticks([])  # 去除横坐标数值
# plt.yticks([])  # 去除纵坐标数值
#
# plt.savefig("KNN_query.pdf")
# plt.show()
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# 生成一些示例数据
# 表A的数据
A_X = np.array([1, 2, 3, 4, 5])
A_Y = np.array([2, 3, 4, 5, 6])

# 表B的数据
B_Z = np.array([1, 2, 3, 4, 5])

# 创建一个三维图形对象
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# 绘制表A的数据点
ax.scatter(A_X, A_Y, np.zeros_like(A_X), c='r', marker='o', label='Table A')

# 绘制表B的数据点
ax.scatter(B_Z, np.zeros_like(B_Z), B_Z, c='b', marker='^', label='Table B')

# 绘制X=Z的平面
X, Z = np.meshgrid(A_X, B_Z)
Y = np.zeros_like(X)  # 在X=Z平面上Y坐标都为0
ax.plot_surface(X, Y, Z, alpha=0.5)

# 设置图形的标签和标题
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
plt.title('3D JOIN Visualization')

# 显示图例
plt.legend()

# 显示图形
plt.show()


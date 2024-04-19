import matplotlib.pyplot as plt

# 索引及其对应的点查询时间数据
index_names = ["rtree", "rstar", "ug", "edg", "zm", "mli", "lisa", "ifi", "flood"]
query_times = [1.09, 8.66, 11.3, 14.26, 0.01, 1.93, 6.45, 0.19, 0.01]  # 这里是示例数据，你需要替换成你自己的数据

# 绘制条形图
plt.figure(figsize=(8, 6))
bars = plt.bar(index_names, query_times, color=(40 / 255, 120 / 255, 181 / 255))

# 在条形图上显示数值
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width() / 2, yval, round(yval, 2), va='bottom')
# 添加标签和标题

plt.ylabel('Avg Query Time (μs)',fontsize=20)
plt.title('Lognormal:Point Query Time',fontsize=32)

plt.savefig("lognormal_point_query.pdf")
# 显示条形图
plt.show()

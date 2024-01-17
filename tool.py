import numpy as np
# 循环次数
times = 10
false_white_class_all = []
miss_white_class_all = []
for index in range(times):
    # 生成数据集
    # predict 为 100 个 1 到 6 的之间的随机数组成的数组
    # real 为 100 个 1 到 6 之间的随机数组成的数组
    predict = np.random.randint(3, 6, 100)
    real = np.random.randint(1, 5, 100)
    # predict = np.array([2, 2, 2, 2, 4, 5, 5, 5, 5, 6, 6, 6])
    # real = np.array([2, 3, 3, 4, 5, 6, 6, 5, 6, 5, 6, 6])
    white_thr = 3
    # 计算漏报和误报
    # 统计 predict 中的数字出现的次数大于 white_thr 次的数字有哪些
    predict_class = np.bincount(predict)
    predict_white_class = np.where(predict_class >= white_thr)
    # 统计 real 中的数字出现的次数大于 white_thr 次的数字有哪些
    real_class = np.bincount(real)
    real_white_class = np.where(real_class >= white_thr)

    # # real_white_class 中有，而 predict_white_class 中没有的类别
    # print("漏报的类别为：", np.setdiff1d(real_white_class, predict_white_class))
    # # predict_white_class 中有，而 real_white_class 中没有的类别
    # print("误报的类别为：", np.setdiff1d(predict_white_class, real_white_class))

    # predict 与 real 的一一对应的点集
    intersect = []
    for i in range(len(predict)):
        if predict[i] == real[i]:
            intersect.append(predict[i])
    intersect = np.array(intersect)
    intersect_white_class = np.where(np.bincount(intersect) >= white_thr)
    false_white_class = np.setdiff1d(predict_white_class, intersect_white_class)
    miss_white_class = np.setdiff1d(real_white_class, intersect_white_class)

    # 数组拼接
    if index == 0:
        false_white_class_all = false_white_class
        miss_white_class_all = miss_white_class
    else:
        false_white_class_all = np.append(false_white_class_all, false_white_class)
        miss_white_class_all = np.append(miss_white_class_all, miss_white_class)

# 统计 false_white_class_all 中各个数字出现次数，存入字典，键值为数字，值为出现次数
false_white_class_all_dict = {}        
for i in range(len(false_white_class_all)):
    if false_white_class_all[i] in false_white_class_all_dict:
        false_white_class_all_dict[false_white_class_all[i]] += 1
    else:
        false_white_class_all_dict[false_white_class_all[i]] = 1      

# 统计 miss_white_class_all 中各个数字出现次数，存入字典，键值为数字，值为出现次数
miss_white_class_all_dict = {}
for i in range(len(miss_white_class_all)):
    if miss_white_class_all[i] in miss_white_class_all_dict:
        miss_white_class_all_dict[miss_white_class_all[i]] += 1
    else:
        miss_white_class_all_dict[miss_white_class_all[i]] = 1

# 字典中所有值除以循环次数
for key in false_white_class_all_dict:
    false_white_class_all_dict[key] /= times
for key in miss_white_class_all_dict:
    miss_white_class_all_dict[key] /= times
# 字典按元素的名称排序
false_white_class_all_dict_sorted = dict(sorted(false_white_class_all_dict.items()))
miss_white_class_all_dict_sorted = dict(sorted(miss_white_class_all_dict.items()))
# 输出两个字典
print("误白：", false_white_class_all_dict_sorted)
print("漏白：", miss_white_class_all_dict_sorted)
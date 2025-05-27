# CDS - Compact Data Structures & Utilities

![License](https://img.shields.io/badge/License-MIT-blue.svg)

CDS 是一个轻量级的 C 语言库，包含三个独立模块：**数据结构库**、**命令行解析工具**和 **OBJ 模型加载器**。

---

## 模块概览

### 1. CDS - 数据结构库
- **核心功能**：提供高效且类型安全的数据结构实现
- **支持类型**：
  - 动态数组（CDS_VECTOR）
  - 链表（CDS_LINK）
  - 队列（CDS_QUEUE）
  - 平衡树（CDS_BRTREE/CDS_AVLTREE）
  - 字符串（CDS_STRING）
- **特性**：
  - 内存自动扩展
  - 支持自定义比较和打印函数
  - 提供迭代和闭包式资源释放

### 2. CCMD - 命令行解析工具
- **核心功能**：简化命令行参数的解析和验证
- **特性**：
  - 自动类型转换（整数、字符串、可变参数列表）
  - 可选/必选参数标记
  - 参数冲突检测
  - 支持回调函数绑定

### 3. OBJ_MODEL - OBJ 模型加载器
- **核心功能**：解析 Wavefront OBJ 文件和关联的 MTL 材质文件
- **支持特性**：
  - 顶点/法线/纹理坐标加载
  - 三角面片索引处理
  - 材质属性解析（漫反射、高光、贴图路径等）
  - 自动处理多边形三角化
  - 支持组（g）和光滑组（s）

---

## 使用示例

### CDS

#### CDS_VECTOR
```c
CDS_VECTOR(int) vec;
cds_vector_init(&vec, sizeof(int));
cds_push(vector, &vec, 42); // 添加元素
printf("Size: %zu\n", vec.size); // 输出: Size: 1
cds_vector_close(&vec, NULL, sizeof(int));
```

### CCMD
```c
#include "ccmd.h"
int main(int argc, char **argv) {
    ccmd_init(argc, (const char**)argv);
    int verbose = 0;
    ccmd_set_para("verbose", CCMD_OPTIONAL | CCMD_FUNCTION, &verbose);
    if (ccmd_deal(1)) return 1; // 自动处理参数
    if (verbose) printf("Verbose mode enabled.\n");
    ccmd_close();
    return 0;
}
```

### OBJ_MODEL
```c
size_t model_count;
OBJ_MTL_ARR *mtl = obj_mtl_create();
OBJ_MODEL *models = obj_load("model.obj", &model_count, mtl);

// 遍历第一个模型的网格
for (size_t i = 0; i < models[0].n; i++) {
    printf("Mesh %zu: %s\n", i, models[0].groups[i].g);
}

obj_unload(models, model_count);
obj_mtl_delete(mtl);
```
---
## 安装

- **安装方式**：直接编译，例如：`gcc -c cds.c`

## 贡献
欢迎提交 Issue 或 Pull Request！请确保：
- 代码符合 C99 标准

---
#### 该README由AI生成

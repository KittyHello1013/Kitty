#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
// 获取当前时间（秒），Windows平台实现
double get_time_sec() {
    return (double)clock() / CLOCKS_PER_SEC;
}
#else
#include <sys/time.h>
// 获取当前时间（秒）
double get_time_sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}
#endif

// 常量定义
#define MAX_N 10000           // 最大物品数量
#define MAX_CAPACITY 1000000  // 最大背包容量
#define TIME_LIMIT 200.0      // 算法超时限制（秒）

// 物品结构体定义
typedef struct {
    int weight;    // 物品重量
    double value;  // 物品价值
} Item;

// 全局变量
Item items[MAX_N];      // 物品数组
int selected[MAX_N];    // 记录选中的物品

double max_value = 0;   // 蛮力法找到的最大价值
double best_value = 0;  // 回溯法找到的最大价值

/**
 * 生成随机物品数据并保存到CSV文件
 * @param n 物品数量
 * @param filename 保存文件名
 */
void generate_and_save_items_csv(int n, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("打开文件失败 %s\n", filename);
        exit(1);
    }
    srand((unsigned int)time(NULL));  // 初始化随机数生成器
    fprintf(fp, "物品编号,物品重量,物品价值\n");
    for (int i = 0; i < n; ++i) {
        int weight = rand() % 100 + 1;  // 随机生成1-100之间的重量
        double value = 100 + rand() % 901 + (rand() % 100) / 100.0;  // 随机生成100-1000之间的价值，保留两位小数
        fprintf(fp, "%d,%d,%.2f\n", i + 1, weight, value);
    }
    fclose(fp);
}

/**
 * 从CSV文件加载物品数据
 * @param filename 文件名
 * @param items 物品数组
 * @param max_n 最大物品数量
 * @return 实际加载的物品数量
 */
int load_items_from_csv(const char* filename, Item* items, int max_n) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("文件读取失败 %s\n", filename);
        return -1;
    }
    char line[128];
    int i = 0;
    fgets(line, sizeof(line), fp);  // 跳过CSV文件的标题行
    while (fgets(line, sizeof(line), fp) && i < max_n) {
        int id, weight;
        double value;
        sscanf(line, "%d,%d,%lf", &id, &weight, &value);  // 从行中解析数据
        items[i].weight = weight;
        items[i].value = value;
        i++;
    }
    fclose(fp);
    return i;
}

/**
 * 动态规划法求解0-1背包问题
 * @param n 物品数量
 * @param capacity 背包容量
 * @return 能获得的最大价值
 */
double dp_knapsack(int n, int capacity) {
    // 创建一维DP数组，dp[w]表示背包容量为w时能获得的最大价值
    double* dp = (double*)calloc(capacity + 1, sizeof(double));

    // 填充DP表
    for (int i = 0; i < n; i++) {
        // 从后向前遍历，避免物品被重复选择
        for (int w = capacity; w >= items[i].weight; w--) {
            double newVal = dp[w - items[i].weight] + items[i].value;
            if (newVal > dp[w]) {
                dp[w] = newVal;
            }
        }
    }

    double result = dp[capacity];
    free(dp);  // 释放动态分配的内存
    return result;
}

/**
 * 蛮力法递归函数
 * @param n 物品数量
 * @param capacity 背包容量
 * @param i 当前考虑的物品索引
 * @param weight 当前已选择物品的总重量
 * @param value 当前已选择物品的总价值
 * @param start_time 算法开始时间，用于超时判断
 */
void brute_force(int n, int capacity, int i, int weight, double value, double start_time) {
    // 如果算法运行时间超过限制，提前返回
    if (get_time_sec() - start_time > TIME_LIMIT) return;

    // 递归终止条件：已经考虑完所有物品
    if (i == n) {
        // 如果当前选择的物品总重量不超过背包容量，且总价值大于已知的最大价值，则更新最大价值
        if (weight <= capacity && value > max_value)
            max_value = value;
        return;
    }

    // 不选择当前物品
    brute_force(n, capacity, i + 1, weight, value, start_time);

    // 选择当前物品
    brute_force(n, capacity, i + 1, weight + items[i].weight, value + items[i].value, start_time);
}

/**
 * 比较函数，用于qsort排序
 * 按照单位重量价值（价值/重量）从大到小排序
 */
int cmp(const void* a, const void* b) {
    double r1 = ((Item*)a)->value / ((Item*)a)->weight;
    double r2 = ((Item*)b)->value / ((Item*)b)->weight;
    return (r2 > r1) - (r2 < r1);  // 返回1表示a应排在b后面，返回-1表示a应排在b前面
}

/**
 * 贪心法求解0-1背包问题
 * @param n 物品数量
 * @param capacity 背包容量
 * @return 能获得的最大价值
 */
double greedy_knapsack(int n, int capacity) {
    // 按照单位重量价值从大到小排序
    qsort(items, n, sizeof(Item), cmp);

    int weight = 0;
    double value = 0;

    // 按价值密度从高到低选择物品
    for (int i = 0; i < n; i++) {
        if (weight + items[i].weight <= capacity) {
            weight += items[i].weight;
            value += items[i].value;
        }
    }

    return value;
}


void backtrack(int n, int idx, int capacity, int weight, double value, double start_time) {
    // 如果算法运行时间超过限制，提前返回
    if (get_time_sec() - start_time > TIME_LIMIT) return;

    // 递归终止条件：已经考虑完所有物品
    if (idx == n) {
        // 如果当前选择的物品总重量不超过背包容量，且总价值大于已知的最大价值，则更新最大价值
        if (weight <= capacity && value > best_value)
            best_value = value;
        return;
    }

    // 选择当前物品（如果重量不超过背包容量）
    if (weight + items[idx].weight <= capacity)
        backtrack(n, idx + 1, capacity, weight + items[idx].weight, value + items[idx].value, start_time);

    // 不选择当前物品
    backtrack(n, idx + 1, capacity, weight, value, start_time);
}

int main() {
    int n, capacity;
    printf("请输入物品数量: ");
    scanf("%d", &n);
    printf("请输入背包容量: ");
    scanf("%d", &capacity);

    // 生成并保存物品数据到CSV文件
    char csv_file[64];
    snprintf(csv_file, sizeof(csv_file), "items_%d.csv", n);
    generate_and_save_items_csv(n, csv_file);

    // 从CSV文件加载物品数据
    if (load_items_from_csv(csv_file, items, MAX_N) <= 0) {
        printf("加载数据失败！\n");
        return 1;
    }

    // 创建结果文件并写入标题行
    FILE* log = fopen("result_summary.csv", "a");
    fprintf(log, "算法,物品数,背包容量,运行时间(ms),总价值\n");

    double t1, t2, result;

    // 动态规划法
    printf("正在运行动态规划法...\n");
    t1 = get_time_sec();
    result = dp_knapsack(n, capacity);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "动态规划,%d,%d,超时,NA\n", n, capacity);
    else
        fprintf(log, "动态规划,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, result);

    // 蛮力法
    printf("正在运行蛮力法...\n");
    t1 = get_time_sec();
    max_value = 0;
    brute_force(n, capacity, 0, 0, 0, t1);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "蛮力法,%d,%d,超时,NA\n", n, capacity);
    else
        fprintf(log, "蛮力法,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, max_value);

    // 贪心法
    printf("正在运行贪心法...\n");
    t1 = get_time_sec();
    result = greedy_knapsack(n, capacity);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "贪心法,%d,%d,超时,NA\n", n, capacity);
    else
        fprintf(log, "贪心法,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, result);

    // 回溯法
    printf("正在运行回溯法...\n");
    t1 = get_time_sec();
    best_value = 0;
    backtrack(n, 0, capacity, 0, 0, t1);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "回溯法,%d,%d,超时,NA\n", n, capacity);
    else
        fprintf(log, "回溯法,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, best_value);

    fclose(log);
    printf("结果已保存至 result_summary.csv\n");
    return 0;
}

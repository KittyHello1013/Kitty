#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
// ��ȡ��ǰʱ�䣨�룩��Windowsƽ̨ʵ��
double get_time_sec() {
    return (double)clock() / CLOCKS_PER_SEC;
}
#else
#include <sys/time.h>
// ��ȡ��ǰʱ�䣨�룩
double get_time_sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}
#endif

// ��������
#define MAX_N 10000           // �����Ʒ����
#define MAX_CAPACITY 1000000  // ��󱳰�����
#define TIME_LIMIT 200.0      // �㷨��ʱ���ƣ��룩

// ��Ʒ�ṹ�嶨��
typedef struct {
    int weight;    // ��Ʒ����
    double value;  // ��Ʒ��ֵ
} Item;

// ȫ�ֱ���
Item items[MAX_N];      // ��Ʒ����
int selected[MAX_N];    // ��¼ѡ�е���Ʒ

double max_value = 0;   // �������ҵ�������ֵ
double best_value = 0;  // ���ݷ��ҵ�������ֵ

/**
 * ���������Ʒ���ݲ����浽CSV�ļ�
 * @param n ��Ʒ����
 * @param filename �����ļ���
 */
void generate_and_save_items_csv(int n, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("���ļ�ʧ�� %s\n", filename);
        exit(1);
    }
    srand((unsigned int)time(NULL));  // ��ʼ�������������
    fprintf(fp, "��Ʒ���,��Ʒ����,��Ʒ��ֵ\n");
    for (int i = 0; i < n; ++i) {
        int weight = rand() % 100 + 1;  // �������1-100֮�������
        double value = 100 + rand() % 901 + (rand() % 100) / 100.0;  // �������100-1000֮��ļ�ֵ��������λС��
        fprintf(fp, "%d,%d,%.2f\n", i + 1, weight, value);
    }
    fclose(fp);
}

/**
 * ��CSV�ļ�������Ʒ����
 * @param filename �ļ���
 * @param items ��Ʒ����
 * @param max_n �����Ʒ����
 * @return ʵ�ʼ��ص���Ʒ����
 */
int load_items_from_csv(const char* filename, Item* items, int max_n) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("�ļ���ȡʧ�� %s\n", filename);
        return -1;
    }
    char line[128];
    int i = 0;
    fgets(line, sizeof(line), fp);  // ����CSV�ļ��ı�����
    while (fgets(line, sizeof(line), fp) && i < max_n) {
        int id, weight;
        double value;
        sscanf(line, "%d,%d,%lf", &id, &weight, &value);  // �����н�������
        items[i].weight = weight;
        items[i].value = value;
        i++;
    }
    fclose(fp);
    return i;
}

/**
 * ��̬�滮�����0-1��������
 * @param n ��Ʒ����
 * @param capacity ��������
 * @return �ܻ�õ�����ֵ
 */
double dp_knapsack(int n, int capacity) {
    // ����һάDP���飬dp[w]��ʾ��������Ϊwʱ�ܻ�õ�����ֵ
    double* dp = (double*)calloc(capacity + 1, sizeof(double));

    // ���DP��
    for (int i = 0; i < n; i++) {
        // �Ӻ���ǰ������������Ʒ���ظ�ѡ��
        for (int w = capacity; w >= items[i].weight; w--) {
            double newVal = dp[w - items[i].weight] + items[i].value;
            if (newVal > dp[w]) {
                dp[w] = newVal;
            }
        }
    }

    double result = dp[capacity];
    free(dp);  // �ͷŶ�̬������ڴ�
    return result;
}

/**
 * �������ݹ麯��
 * @param n ��Ʒ����
 * @param capacity ��������
 * @param i ��ǰ���ǵ���Ʒ����
 * @param weight ��ǰ��ѡ����Ʒ��������
 * @param value ��ǰ��ѡ����Ʒ���ܼ�ֵ
 * @param start_time �㷨��ʼʱ�䣬���ڳ�ʱ�ж�
 */
void brute_force(int n, int capacity, int i, int weight, double value, double start_time) {
    // ����㷨����ʱ�䳬�����ƣ���ǰ����
    if (get_time_sec() - start_time > TIME_LIMIT) return;

    // �ݹ���ֹ�������Ѿ�������������Ʒ
    if (i == n) {
        // �����ǰѡ�����Ʒ�������������������������ܼ�ֵ������֪������ֵ�����������ֵ
        if (weight <= capacity && value > max_value)
            max_value = value;
        return;
    }

    // ��ѡ��ǰ��Ʒ
    brute_force(n, capacity, i + 1, weight, value, start_time);

    // ѡ��ǰ��Ʒ
    brute_force(n, capacity, i + 1, weight + items[i].weight, value + items[i].value, start_time);
}

/**
 * �ȽϺ���������qsort����
 * ���յ�λ������ֵ����ֵ/�������Ӵ�С����
 */
int cmp(const void* a, const void* b) {
    double r1 = ((Item*)a)->value / ((Item*)a)->weight;
    double r2 = ((Item*)b)->value / ((Item*)b)->weight;
    return (r2 > r1) - (r2 < r1);  // ����1��ʾaӦ����b���棬����-1��ʾaӦ����bǰ��
}

/**
 * ̰�ķ����0-1��������
 * @param n ��Ʒ����
 * @param capacity ��������
 * @return �ܻ�õ�����ֵ
 */
double greedy_knapsack(int n, int capacity) {
    // ���յ�λ������ֵ�Ӵ�С����
    qsort(items, n, sizeof(Item), cmp);

    int weight = 0;
    double value = 0;

    // ����ֵ�ܶȴӸߵ���ѡ����Ʒ
    for (int i = 0; i < n; i++) {
        if (weight + items[i].weight <= capacity) {
            weight += items[i].weight;
            value += items[i].value;
        }
    }

    return value;
}


void backtrack(int n, int idx, int capacity, int weight, double value, double start_time) {
    // ����㷨����ʱ�䳬�����ƣ���ǰ����
    if (get_time_sec() - start_time > TIME_LIMIT) return;

    // �ݹ���ֹ�������Ѿ�������������Ʒ
    if (idx == n) {
        // �����ǰѡ�����Ʒ�������������������������ܼ�ֵ������֪������ֵ�����������ֵ
        if (weight <= capacity && value > best_value)
            best_value = value;
        return;
    }

    // ѡ��ǰ��Ʒ�������������������������
    if (weight + items[idx].weight <= capacity)
        backtrack(n, idx + 1, capacity, weight + items[idx].weight, value + items[idx].value, start_time);

    // ��ѡ��ǰ��Ʒ
    backtrack(n, idx + 1, capacity, weight, value, start_time);
}

int main() {
    int n, capacity;
    printf("��������Ʒ����: ");
    scanf("%d", &n);
    printf("�����뱳������: ");
    scanf("%d", &capacity);

    // ���ɲ�������Ʒ���ݵ�CSV�ļ�
    char csv_file[64];
    snprintf(csv_file, sizeof(csv_file), "items_%d.csv", n);
    generate_and_save_items_csv(n, csv_file);

    // ��CSV�ļ�������Ʒ����
    if (load_items_from_csv(csv_file, items, MAX_N) <= 0) {
        printf("��������ʧ�ܣ�\n");
        return 1;
    }

    // ��������ļ���д�������
    FILE* log = fopen("result_summary.csv", "a");
    fprintf(log, "�㷨,��Ʒ��,��������,����ʱ��(ms),�ܼ�ֵ\n");

    double t1, t2, result;

    // ��̬�滮��
    printf("�������ж�̬�滮��...\n");
    t1 = get_time_sec();
    result = dp_knapsack(n, capacity);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "��̬�滮,%d,%d,��ʱ,NA\n", n, capacity);
    else
        fprintf(log, "��̬�滮,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, result);

    // ������
    printf("��������������...\n");
    t1 = get_time_sec();
    max_value = 0;
    brute_force(n, capacity, 0, 0, 0, t1);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "������,%d,%d,��ʱ,NA\n", n, capacity);
    else
        fprintf(log, "������,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, max_value);

    // ̰�ķ�
    printf("��������̰�ķ�...\n");
    t1 = get_time_sec();
    result = greedy_knapsack(n, capacity);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "̰�ķ�,%d,%d,��ʱ,NA\n", n, capacity);
    else
        fprintf(log, "̰�ķ�,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, result);

    // ���ݷ�
    printf("�������л��ݷ�...\n");
    t1 = get_time_sec();
    best_value = 0;
    backtrack(n, 0, capacity, 0, 0, t1);
    t2 = get_time_sec();
    if (t2 - t1 > TIME_LIMIT)
        fprintf(log, "���ݷ�,%d,%d,��ʱ,NA\n", n, capacity);
    else
        fprintf(log, "���ݷ�,%d,%d,%.2f,%.2f\n", n, capacity, (t2 - t1) * 1000, best_value);

    fclose(log);
    printf("����ѱ����� result_summary.csv\n");
    return 0;
}

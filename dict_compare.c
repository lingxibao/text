// dict_compare.c
// 字典: di.csv（A列对应C03C，B列对应C04N）
// 功能: 检查 hqmsts_.csv 的 C03C、C04N 两列的每个值是否命中字典，生成质控报告
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 65536
#define FIELD_MAX     1000
#define FIELD_MAX_LEN  128
#define HASH_SIZE   262144

/* ---------- 哈希字典 ---------- */
typedef struct Node {
    char *val;
    struct Node *next;
} Node;

static Node *bucketA[HASH_SIZE];   // C03C 字典（di.csv 的 A 列）
static Node *bucketB[HASH_SIZE];   // C04N 字典（di.csv 的 B 列）

static unsigned hash_str(const char *s) {
    unsigned h = 5381;
    while (*s) h = h * 33u + (unsigned char)*s++;
    return h & (HASH_SIZE - 1);
}

static void dict_add(Node *t[], const char *val) {
    if (!val[0]) return;
    unsigned h = hash_str(val);
    for (Node *p = t[h]; p; p = p->next)
        if (!strcmp(p->val, val)) return;      // 去重
    Node *nd = malloc(sizeof(Node));
    nd->val = malloc(strlen(val) + 1);
    strcpy(nd->val, val);
    nd->next = t[h];
    t[h] = nd;
}

static int dict_has(Node *t[], const char *val) {
    if (!val[0]) return 0;
    for (Node *p = t[hash_str(val)]; p; p = p->next)
        if (!strcmp(p->val, val)) return 1;
    return 0;
}

/* ---------- csv 基础处理 ---------- */
static char fields[FIELD_MAX][FIELD_MAX_LEN];

static void clean_line(char *s) {
    size_t len = strlen(s);
    while (len && (s[len-1]=='\n' || s[len-1]=='\r')) s[--len] = '\0';
    if ((unsigned char)s[0]==0xEF && (unsigned char)s[1]==0xBB && (unsigned char)s[2]==0xBF)
        memmove(s, s+3, len-2);
}

static int split(char *line, char f[][FIELD_MAX_LEN]) {
    int n = 0;
    char *tok = strtok(line, ",");
    while (tok && n < FIELD_MAX) {
        while (*tok==' '||*tok=='\t') tok++;
        char *e = tok + strlen(tok);
        while (e>tok && (e[-1]==' '||e[-1]=='\t')) *--e = '\0';
        strncpy(f[n], tok, FIELD_MAX_LEN-1); f[n][FIELD_MAX_LEN-1]='\0';
        n++;
        tok = strtok(NULL, ",");
    }
    return n;
}

int main(void) {
    const char *dict_file  = "di.csv";
    const char *data_file  = "hqmsts_.csv";
    const char *report_name = "C03C_C04N_质控报告.txt";

    /* 第 1 步：载入 di.csv 的 A、B 列建字典 */
    FILE *fr = fopen(dict_file, "r");                      // 
    if (!fr) { printf("无法打开字典文件 %s\n", dict_file); return 1; }
    char line[LINE_MAX_LEN];
    long dict_rows = 0;
    while (fgets(line, sizeof(line), fr)) {                // 
        clean_line(line);
        int n = split(line, fields);
        if (n > 0) dict_add(bucketA, fields[0]);
        if (n > 1) dict_add(bucketB, fields[1]);
        dict_rows++;
    }
    fclose(fr);
    printf("字典载入完成: %s 共 %ld 行\n", dict_file, dict_rows);

    /* 第 2 步：读取数据表头，定位 C03C、C04N */
    FILE *fd = fopen(data_file, "r");
    if (!fd) { printf("无法打开 %s\n", data_file); return 1; }
    if (!fgets(line, sizeof(line), fd)) { printf("%s 为空\n", data_file); return 1; }
    clean_line(line);
    int ncols = split(line, fields);
    int cA = -1, cB = -1;
    for (int i = 0; i < ncols; i++) {
        if (!strcmp(fields[i], "C03C")) cA = i;
        if (!strcmp(fields[i], "C04N")) cB = i;
    }
    if (cA < 0 || cB < 0) {
        printf("表头缺少 C03C(%d) 或 C04N(%d)\n", cA, cB);
        return 1;
    }
    printf("C03C = 第 %d 列, C04N = 第 %d 列，开始校验...\n", cA + 1, cB + 1);

    /* 第 3 步：逐行查字典 */
    FILE *rep = fopen(report_name, "w");
    if (!rep) { printf("无法创建报告文件\n"); return 1; }
    fprintf(rep, "======== C03C/C04N 字典校验报告 ========\n");
    fprintf(rep, "数据文件: %s\n字典文件: di.csv（A列=C03C基准, B列=C04N基准）\n\n",
            data_file);

    long total = 0, badA = 0, badB = 0;
    int rownum = 1;
    char buf[1024];
    while (fgets(line, sizeof(line), fd)) {
        clean_line(line);
        int n = split(line, fields);
        rownum++; total++;

        int okA = (n > cA) && dict_has(bucketA, fields[cA]);
        int okB = (n > cB) && dict_has(bucketB, fields[cB]);

        if (!okA) {
            snprintf(buf, sizeof(buf), "第 %d 行: C03C=\"%s\" 不在字典中\n",
                     rownum, n > cA ? fields[cA] : "(缺失)");
            fputs(buf, rep); fputs(buf, stdout); badA++;
        }
        if (!okB) {
            snprintf(buf, sizeof(buf), "第 %d 行: C04N=\"%s\" 不在字典中\n",
                     rownum, n > cB ? fields[cB] : "(缺失)");
            fputs(buf, rep); fputs(buf, stdout); badB++;
        }
    }
    fclose(fd);

    fprintf(rep, "\n==== 汇总 ====\n数据行数: %ld\n", total);
    fprintf(rep, "C03C 不在字典: %ld 行\nC04N 不在字典: %ld 行\n", badA, badB);
    fprintf(rep, "结论: %s\n",
            (badA == 0 && badB == 0) ? "全部一致，质控通过" : "存在不一致，见明细");
    fclose(rep);

    printf("\n==== 汇总 ====\n数据行数: %ld  C03C异常: %ld  C04N异常: %ld\n",
           total, badA, badB);
    printf("报告已写入: %s\n", report_name);
    return (badA || badB) ? 1 : 0;
}
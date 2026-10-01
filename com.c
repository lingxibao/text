// compare_header.c
// 对比 hqmsts_对接模板.csv 与 hqmsts_.csv 的表头，不一致时生成表头质控报告
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 65536    // 811 列的表头行可能很长，取 64KB
#define FIELD_MAX     1000
#define FIELD_MAX_LEN  128

static char tf[FIELD_MAX][FIELD_MAX_LEN];   // 模板表头字段
static char df[FIELD_MAX][FIELD_MAX_LEN];   // 目标表头字段

// 去掉行尾 \r\n 及 UTF-8 BOM（Excel 导出常见）
static void clean_line(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) s[--len] = '\0';
    if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB
        && (unsigned char)s[2] == 0xBF)
        memmove(s, s + 3, len - 2);
}

// 按逗号拆分表头到 fields，返回列数
static int split_header(char *line, char fields[][FIELD_MAX_LEN]) {
    int n = 0;
    char *tok = strtok(line, ",");
    while (tok && n < FIELD_MAX) {
        while (*tok == ' ' || *tok == '\t') tok++;
        char *end = tok + strlen(tok);
        while (end > tok && (end[-1]==' ' || end[-1]=='\t')) *--end = '\0';
        strncpy(fields[n], tok, FIELD_MAX_LEN - 1);
        fields[n][FIELD_MAX_LEN - 1] = '\0';
        n++;
        tok = strtok(NULL, ",");
    }
    return n;
}

int main(void) {
    const char *tpl  = "hqmsts_对接模板.csv";
    const char *dst  = "hqmsts_.csv";
    const char *report_name = "表头质控报告.txt";

    FILE *ft = fopen(tpl, "r");                 // 
    if (!ft) { printf("无法打开 %s\n", tpl); return 1; }
    FILE *fd = fopen(dst, "r");
    if (!fd) { printf("无法打开 %s\n", dst); fclose(ft); return 1; }

    static char line_t[LINE_MAX_LEN], line_d[LINE_MAX_LEN];
    if (!fgets(line_t, sizeof(line_t), ft) ||   // 
        !fgets(line_d, sizeof(line_d), fd)) {
        printf("某个文件为空，无法读取表头\n");
        return 1;
    }
    clean_line(line_t);
    clean_line(line_d);
    fclose(ft);
    fclose(fd);

    int nt = split_header(line_t, tf);
    int nd = split_header(line_d, df);

    int mismatch = 0, diff_cnt = 0;
    int max_col = (nt > nd) ? nt : nd;
    char buf[1024];

    // 先扫描一遍，统计差异数，确定是否需要生成报告
    for (int i = 0; i < max_col; i++) {
        if (i < nt && i < nd) {
            if (strcmp(tf[i], df[i]) != 0) diff_cnt++;      // strcmp 相同为 0 
        } else diff_cnt++;                                   // 某方缺列
    }

    if (diff_cnt == 0) {
        printf("表头一致：共 %d 列\n", nt);
        return 0;
    }

    // 不一致 → 生成质控报告
    FILE *rep = fopen(report_name, "w");
    if (!rep) { printf("无法创建 %s\n", report_name); return 1; }

    fprintf(rep, "======== 表头质控报告 ========\n");
    fprintf(rep, "模板文件: %s  (%d 列)\n", tpl, nt);
    fprintf(rep, "目标文件: %s  (%d 列)\n", dst, nd);
    fprintf(rep, "差异列数: %d\n\n", diff_cnt);

    for (int i = 0; i < max_col; i++) {
        if (i < nt && i < nd) {
            if (strcmp(tf[i], df[i]) != 0) {
                snprintf(buf, sizeof(buf),
                         "第 %d 列不一致: 模板=\"%s\" 实际=\"%s\"\n",
                         i + 1, tf[i], df[i]);
                fputs(buf, rep);                             // 
                printf("%s", buf);
                mismatch = 1;
            }
        } else if (i < nt) {
            snprintf(buf, sizeof(buf), "第 %d 列: 目标文件缺少 \"%s\"\n", i + 1, tf[i]);
            fputs(buf, rep); printf("%s", buf); mismatch = 1;
        } else {
            snprintf(buf, sizeof(buf), "第 %d 列: 模板中不存在 \"%s\"\n", i + 1, df[i]);
            fputs(buf, rep); printf("%s", buf); mismatch = 1;
        }
    }

    fprintf(rep, "\n结论: 表头不一致，请修正 %s 后重新校验。\n", dst);
    fclose(rep);

    printf("\n共 %d 处差异，报告已写入: %s\n", diff_cnt, report_name);
    return 1;    // 返回 1 表示不一致，供脚本判断
}
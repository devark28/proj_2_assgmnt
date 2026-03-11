#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DELIM " \t\n\r.,!?;:\"'()[]{}/<>-_"

typedef struct {
    char *word;
    int length;
} Word;

typedef struct {
    Word *words;
    int count, capacity;
} Tbl;

char *slurp(void) {
    char *buf = malloc(256);
    int cap = 256, len = 0, c;
    puts("Paste text, then press Ctrl+D:");
    while ((c = getchar()) != EOF) {
        if (len + 2 >= cap) buf = realloc(buf, cap *= 2);
        buf[len++] = (char) c;
    }
    buf[len] = '\0';
    return buf;
}

void to_lower(char *s) {
    for (; *s; s++)
        *s = (char) tolower((unsigned char) *s);
}

Tbl *tbl_new(void) {
    Tbl *t = malloc(sizeof(Tbl));
    t->words = malloc(16 * sizeof(Word));
    t->count = 0;
    t->capacity = 16;
    return t;
}

void tbl_insert(Tbl *t, const char *w) {
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->words[i].word, w) == 0) {
            t->words[i].length++;
            return;
        }
    }
    if (t->count == t->capacity) {
        t->capacity *= 2;
        t->words = realloc(t->words, t->capacity * sizeof(Word));
    }
    t->words[t->count].word = strdup(w);
    t->words[t->count].length = 1;
    t->count++;
}

void tbl_free(Tbl *t) {
    for (int i = 0; i < t->count; i++) free(t->words[i].word);
    free(t->words);
    free(t);
}

Tbl *build_freq(const char *text, int *total) {
    Tbl *t = tbl_new();
    char *tmp = strdup(text);
    to_lower(tmp);
    *total = 0;
    char *tok = strtok(tmp, DELIM);
    while (tok) {
        tbl_insert(t, tok);
        (*total)++;
        tok = strtok(NULL, DELIM);
    }
    free(tmp);
    return t;
}

int cmp_count(const void *a, const void *b) {
    return ((Word *) b)->length - ((Word *) a)->length;
}

void show_top(Tbl *t, int k) {
    qsort(t->words, t->count, sizeof(Word), cmp_count);
    int lim = t->count < k ? t->count : k;
    printf("\n  Top %d words:\n", lim);
    for (int i = 0; i < lim; i++)
        printf("  %-24s %d\n", t->words[i].word, t->words[i].length);
}

void find_longest(Tbl *t) {
    int mx = 0, idx = 0;
    for (int i = 0; i < t->count; i++) {
        int len = (int) strlen(t->words[i].word);
        if (len > mx) {
            mx = len;
            idx = i;
        }
    }
    printf("Longest word: \"%s\" (%d chars)\n", t->words[idx].word, mx);
}

int count_syllables(const char *w) {
    int count = 0, in_v = 0;
    for (const char *p = w; *p; p++) {
        int v = strchr("aeiou", tolower((unsigned char) *p)) != NULL;
        if (v && !in_v)
            count++;
        in_v = v;
    }
    return count < 1 ? 1 : count;
}

void analyze(const char *text, int total, int unique) {
    int sentences = 0, syllables = 0;
    for (const char *p = text; *p; p++)
        if (*p == '.' || *p == '!' || *p == '?')
            sentences++;

    char *tmp = strdup(text);
    to_lower(tmp);
    char *tok = strtok(tmp, DELIM);
    while (tok) {
        syllables += count_syllables(tok);
        tok = strtok(NULL, DELIM);
    }
    free(tmp);

    printf("\nTotal words:\t%d\n", total);
    printf("Unique words:\t%d\n", unique);
    printf("Sentences:\t%d\n", sentences);
    printf("Syllables:\t%d\n", syllables);

    if (total > 0 && sentences > 0) {
        float avg = (float) syllables / total;
        float total_by_sents = (float) total / sentences;
        float fk = 206.835f - 1.015f * total_by_sents - 84.6f * avg;
        printf("Flesch score: %.1f", fk);
        if (fk >= 70)
            puts(" (Easy)");
        else if (fk >= 50)
            puts(" (Standard)");
        else
            puts(" (Difficult)");
    }
}

int main(void) {
    char *text = slurp();
    if (!*text) {
        puts("No input.");
        free(text);
        return 1;
    }

    int total;
    Tbl *t = build_freq(text, &total);

    analyze(text, total, t->count);
    show_top(t, 10);
    find_longest(t);

    tbl_free(t);
    free(text);
    return 0;
}

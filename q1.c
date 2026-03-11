#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    int id;
    float gpa;
    int year;
} Student;

typedef struct {
    Student *students;
    int count, capacity;
} DB;

typedef void (*Op)(DB *);

DB *db_new(void) {
    DB *db = malloc(sizeof(DB));
    db->students = malloc(4 * sizeof(Student));
    db->count = 0;
    db->capacity = 4;
    return db;
}

void db_grow(DB *db) {
    db->capacity *= 2;
    Student *tmp = realloc(db->students, db->capacity * sizeof(Student));
    if (!tmp) {
        fputs("Out of memory.\n", stderr);
        exit(1);
    }
    db->students = tmp;
}

void db_free(DB *db) {
    free(db->students);
    free(db);
}

void add(DB *db) {
    if (db->count == db->capacity) db_grow(db);
    Student *s = db->students + db->count;
    char line[128];
    printf("Name: ");
    fgets(line, sizeof line, stdin);
    sscanf(line, "%63s", s->name);
    printf("ID: ");
    fgets(line, sizeof line, stdin);
    sscanf(line, "%d", &s->id);
    printf("GPA: ");
    fgets(line, sizeof line, stdin);
    sscanf(line, "%f", &s->gpa);
    printf("Year: ");
    fgets(line, sizeof line, stdin);
    sscanf(line, "%d", &s->year);
    db->count++;
    puts("Added.");
}

void display(DB *db) {
    if (!db->count) {
        puts("No records.");
        return;
    }
    printf("\n%-22s %6s %4s %s\n", "Name", "ID", "Year", "GPA");
    printf("%-22s %6s %4s ---\n", "----", "--", "----");
    for (Student *p = db->students, *e = p + db->count; p < e; p++)
        printf("%-22s %06d %4d %.2f\n", p->name, p->id, p->year, p->gpa);
}

int cmp_gpa(const void *a, const void *b) {
    float d = ((Student *) b)->gpa - ((Student *) a)->gpa;
    return (d > 0) - (d < 0);
}

int cmp_name(const void *a, const void *b) {
    return strcmp(((Student *) a)->name, ((Student *) b)->name);
}

void by_gpa(DB *db) {
    qsort(db->students, db->count, sizeof(Student), cmp_gpa);
    puts("Sorted by GPA (highest first).");
    display(db);
}

void by_name(DB *db) {
    qsort(db->students, db->count, sizeof(Student), cmp_name);
    puts("Sorted by name (A-Z).");
    display(db);
}

void analyze(DB *db) {
    if (!db->count) {
        puts("No data.");
        return;
    }
    double sum = 0;
    int pass = 0;
    Student *hi = db->students, *lo = db->students;
    for (Student *p = db->students, *e = p + db->count; p < e; p++) {
        sum += p->gpa;
        if (p->gpa > hi->gpa) hi = p;
        if (p->gpa < lo->gpa) lo = p;
        if (p->gpa >= 2.0f) pass++;
    }
    printf("\tCount:\t%d\n", db->count);
    printf("\tAverage:\t%.2f\n", sum / db->count);
    printf("\tTop:\t%s (%.2f)\n", hi->name, hi->gpa);
    printf("\tBottom:\t%s (%.2f)\n", lo->name, lo->gpa);
    printf("\tPass rate:\t%d/%d (%.0f%%)\n", pass, db->count, 100.0 * pass / db->count);
}

int main(void) {
    DB *db = db_new();

    Op ops[] = {add, display, by_gpa, by_name, analyze};
    char *labels[] = {"Add", "Display", "ByGPA", "ByName", "Analyze"};
    int ch;
    char line[16];

    for (;;) {
        printf("\n");
        for (int i = 0; i < 5; i++)
            printf("%d)%s  ", i + 1, labels[i]);
        printf("0)Exit\n> ");
        fgets(line, sizeof line, stdin);
        if (sscanf(line, "%d", &ch) != 1)
            continue;
        if (ch == 0)
            break;
        if (ch >= 1 && ch <= 5)
            ops[ch - 1](db);
    }

    db_free(db);
    return 0;
}

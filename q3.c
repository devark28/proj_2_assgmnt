#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SensorsCount 4

typedef enum { TEMP = 0, HUMID, PRES, VOLT } SensorType;

const char *stype_name[] = {"temperature", "humidity", "pressure", "voltage"};

typedef union {
    float fval;
    int ival;
} Val;

typedef struct {
    char id[24];
    SensorType type;
    Val reading;
} Sensor;

typedef void (*CBFn)(const Sensor *);

typedef struct {
    CBFn fn;
    char name[32];
} CB;

typedef struct {
    CB *list;
    int count, capacity;
} Hub;

Hub hub = {NULL, 0, 0};

void hub_add(CBFn fn, const char *name) {
    if (hub.count == hub.capacity) {
        hub.capacity = hub.capacity ? hub.capacity * 2 : 4;
        hub.list = realloc(hub.list, hub.capacity * sizeof(CB));
    }
    hub.list[hub.count].fn = fn;
    strncpy(hub.list[hub.count].name, name, 31);
    hub.list[hub.count].name[31] = '\0';
    hub.count++;
}

void hub_fire(const Sensor *s) {
    for (int i = 0; i < hub.count; i++)
        hub.list[i].fn(s);
}

void cb_log(const Sensor *s) {
    printf("[LOG] %-14s  %s = ", s->id, stype_name[s->type]);
    if (s->type == PRES)
        printf("%d hPa\n", s->reading.ival);
    else
        printf("%.2f\n", s->reading.fval);
}

void cb_alert(const Sensor *s) {
    int bad = 0;
    if (s->type == TEMP && s->reading.fval > 40.0f)
        bad = 1;
    if (s->type == HUMID && s->reading.fval > 85.0f)
        bad = 1;
    if (s->type == PRES && s->reading.ival > 1050)
        bad = 1;
    if (s->type == VOLT && s->reading.fval < 3.0f)
        bad = 1;
    if (bad)
        printf("[ALERT] %-14s threshold exceeded!\n", s->id);
}

typedef struct {
    double sum;
    float hi, lo;
    int count;
} Stat;

Stat stats[SensorsCount];
int st_ready = 0;

void cb_update_stats(const Sensor *s) {
    if (!st_ready) {
        for (int i = 0; i < SensorsCount; i++)
            stats[i] = (Stat){0, -1e9f, 1e9f, 0};
        st_ready = 1;
    }
    float v = (s->type == PRES) ? (float) s->reading.ival : s->reading.fval;
    Stat *st = &stats[s->type];
    st->sum += v;
    st->count++;
    if (v > st->hi)
        st->hi = v;
    if (v < st->lo)
        st->lo = v;
}

void print_stats(void) {
    puts("\nSession Stats");
    for (int k = 0; k < 4; k++) {
        if (!stats[k].count)
            continue;
        printf("%-12s n=%d avg=%.2f hi=%.2f lo=%.2f\n", stype_name[k], stats[k].count,
               stats[k].sum / stats[k].count, stats[k].hi, stats[k].lo);
    }
}

Sensor make_sensor(const char *id, SensorType t, float v) {
    Sensor s;
    strncpy(s.id, id, 23);
    s.id[23] = '\0';
    s.type = t;
    if (t == PRES)
        s.reading.ival = (int) v;
    else
        s.reading.fval = v;
    return s;
}

void run_sim(void) {
    typedef struct {
        const char *id;
        SensorType t;
        float v;
    } SubSensor;
    SubSensor fx[] = {
        {"thermo-01", TEMP, 22.5f},
        {"thermo-02", TEMP, 41.8f},
        {"thermo-03", TEMP, 28.8f},
        {"humid-01", HUMID, 60.0f},
        {"humid-02", HUMID, 88.3f},
        {"baro-01", PRES, 1013.0f},
        {"baro-02", PRES, 1055.0f},
        {"baro-03", PRES, 1073.0f},
        {"volt-01", VOLT, 3.7f},
        {"volt-02", VOLT, 2.8f},
    };
    int sz = (int) (sizeof fx / sizeof fx[0]);
    printf("%d sensors", sz);
    puts("Built-in simulation");
    for (int i = 0; i < sz; i++) {
        Sensor s = make_sensor(fx[i].id, fx[i].t, fx[i].v);
        hub_fire(&s);
    }
}

void add_reading(void) {
    Sensor s;
    char line[64];
    int t;
    printf("Device ID: ");
    fgets(line, sizeof line, stdin);
    sscanf(line, "%23s", s.id);
    s.id[23] = '\0';
    printf("Type (0=temp 1=humid 2=pres 3=volt): ");
    fgets(line, sizeof line, stdin);
    if (sscanf(line, "%d", &t) != 1 || t < 0 || t > 3) {
        puts("Invalid type.");
        return;
    }
    s.type = (SensorType) t;
    if (s.type == PRES) {
        printf("Value (int hPa): ");
        fgets(line, sizeof line, stdin);
        sscanf(line, "%d", &s.reading.ival);
    } else {
        float f;
        printf("Value (float): ");
        fgets(line, sizeof line, stdin);
        sscanf(line, "%f", &f);
        s.reading.fval = f;
    }
    hub_fire(&s);
}

int main(void) {
    hub_add(cb_log, "logger");
    hub_add(cb_alert, "alerter");
    hub_add(cb_update_stats, "stats");

    run_sim();

    int ch;
    char line[16];
    do {
        printf("\n1)Add reading  2)Show stats  0)Exit\n> ");
        fgets(line, sizeof line, stdin);
        if (sscanf(line, "%d", &ch) != 1)
            ch = -1;
        if (ch == 1)
            add_reading();
        if (ch == 2)
            print_stats();
    } while (ch != 0);

    print_stats();
    free(hub.list);
    return 0;
}

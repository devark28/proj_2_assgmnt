# Project 2_Assignment

### 1. Dynamic Student Record Engine

A small in-memory student database.
<br/>You can add students through a menu, and the program grows the storage automatically.

**What is happening**

- Keeps a heap-allocated array of `Student` structs that doubles in capacity when it's full
- it lets you sort the list by descending GPA or alphabetically by name using `qsort`
- menu itself is driven by an array of function pointers. option 3 calls `by_gpa`, 5 calls `analyze`, and a few others. No `switch` block needed.
- The analysis report shows count, average gpa, the top and bottom student (found by walking the array with a `Student *p` pointer), and the pass rate.

**Running it:** `./q1`

---

### 2. Adaptive Text Intelligence Tool

Reads any text from standard input and tells you things like word frequencies, basic readability, complexity, adn more
It uses dynamically allocated memory to fit whatever you throw at it.

**What is happening:**

- Reads from stdin byte by byte into a buffer that doubles whenever it runs out of room to works on text of any length
- it tokenizes a lowercase copy of the input using `strtok`, building a frequency table where each unique word has its own heap-allocated string (via `strdup`). The table itself also grows with `realloc` as new words appear.
- Reports total word count, unique word count, sentence count, and syllable count.
- Syllables are estimated by walking each word character by character and counting vowel groups (a simple pointer loop, no indexes)
- a Flesch reading-ease score is computed from those three numbers and labels resulting Easy / Standard / Difficult.
- Shows the top 10 most frequent words and the longest word found.

**Running it:** `./q2`. Paste or type some text, then press `Ctrl+D` to signal end of input.
<br/>Apparently it works well with redirected files too: `./q2 < some_article.txt`

---

### 3. Callback-Based Device Monitoring Simulator

A small event hub that fires registered callbacks whenever a sensor reading comes in.
<br/>The sensors use a tagged union to hold different value types in the same memory footprint.

**What is happening**

- Each `Sensor` has a type tag (`TEMP`, `HUMID`, `PRES`, `VOLT`) and a `Val` union that holds either a `float` or an `int` depending on the type.
- The callbacks always read the correct union member by switching on sensor type. that's the discipline required to use unions safely.
- The `Hub` is a heap-allocated dynamically-sized array of `CB` entries, each holding a function pointer and a name. `hub_fire` loops over every entry and calls each one in turn.
- Three callbacks are registered by default: `cb_log` prints the reading, `cb_alert` checks thresholds and prints a warning if exceeded, and `cb_update_stats` accumulates running min/max/average per sensor type.
- At startup the program runs a built-in simulation with ten pre-defined readings (two/three per sensor type), to see all three callbacks firing immediately. then add i can add my own readings through the menu.

**Running it:** `./q3`

The simulation output appears first, then you get a menu to add more readings manually or view the session statistics.

**Thresholds used by the alert callback**

| Type        | Alert condition          |
|-------------|--------------------------|
| Temperature | above 40 °C              |
| Humidity    | above 85 %               |
| Pressure    | above 1050 hPa           |
| Voltage     | below 3.0 V              |

---

## Notes

- All three programs use `fgets` + `sscanf` for input rather than raw `scanf`, avoiding the classic leftover-newline problem and keeps the input loop predictable.
- `realloc` return values are always captured in a temporary pointer before overwriting the original. so a failed allocation does not silently leak the old block.
- No global state except the `Hub` and `Stat` arrays in q3, which are static to keep the callback signatures simple (callbacks take only a `const Sensor *` because adding a context pointer would require a different design).

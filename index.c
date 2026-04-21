#include "index.h"
#include "pes.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ─── LOAD ─────────────────────────────────────────
int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(INDEX_FILE, "r");
    if (!f) return 0;

    while (1) {
        IndexEntry entry;
        char hash_hex[65];

        if (fscanf(f, "%o %64s %ld %u %255s\n",
                   &entry.mode,
                   hash_hex,
                   &entry.mtime_sec,
                   &entry.size,
                   entry.path) != 5)
            break;

        hex_to_hash(hash_hex, &entry.hash);
        index->entries[index->count++] = entry;
    }

    fclose(f);
    return 0;
}

// ─── SAVE ─────────────────────────────────────────
int index_save(const Index *index) {
    FILE *f = fopen(INDEX_FILE, "w");
    if (!f) return -1;

    for (int i = 0; i < index->count; i++) {
        char hash_hex[65];
        hash_to_hex(&index->entries[i].hash, hash_hex);

        fprintf(f, "%o %s %ld %u %s\n",
                index->entries[i].mode,
                hash_hex,
                index->entries[i].mtime_sec,
                index->entries[i].size,
                index->entries[i].path);
    }

    fclose(f);
    return 0;
}

// ─── ADD ─────────────────────────────────────────
int index_add(Index *index, const char *path) {
    struct stat st;

    if (stat(path, &st) < 0) return -1;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    char *data = malloc(st.st_size);
    fread(data, 1, st.st_size, f);
    fclose(f);

    ObjectID id;
    if (object_write(OBJ_BLOB, data, st.st_size, &id) < 0) {
        free(data);
        return -1;
    }

    free(data);

    IndexEntry *existing = index_find(index, path);

    IndexEntry entry;
    entry.mode = get_file_mode(path);
    entry.hash = id;
    entry.mtime_sec = st.st_mtime;
    entry.size = st.st_size;

    strncpy(entry.path, path, sizeof(entry.path));
    entry.path[sizeof(entry.path)-1] = '\0';

    if (existing) {
        *existing = entry;
    } else {
        index->entries[index->count++] = entry;
    }

    // 🔥 CRITICAL FIX — SAVE AFTER ADD
    return index_save(index);
}

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
  Entry* tombstone = NULL;

  for (uint32_t index = key->hash; ; index++) {
    Entry* entry = &entries[index %= capacity];
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // empty entry
        return tombstone != NULL ? tombstone : entry;
      } else {
        // tombstone value
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) return entry;
  }
}

static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) continue;

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableGet(Table* table, ObjString* key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  *value = entry->value;
  return true;
}

bool tableSet(Table* table, ObjString* key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NIL(entry->value)) table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  entry->key = NULL;
  entry->value = BOOL_VAL(true);

  return true;
}

void tableAddAll(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}


ObjString* tableFindString(Table* table, const char* chars, int length,
    uint32_t hash) {
  if (table->count == 0) return NULL;

  for (uint32_t index = hash; ; index++) {
    Entry* entry = &table->entries[index %= table->capacity];

    if (entry->key == NULL) {
      // if not a tombstone, return
      if (IS_NIL(entry->value)) return NULL;
    } else if (
        // first, just compare the length and hash (cheap)
        entry->key->length == length &&
        entry->key->hash == hash &&
        // then compare the characters themselves (just in case)
        memcmp(entry->key->chars, chars, length) == 0) {
      return entry->key;
    }
  }
}

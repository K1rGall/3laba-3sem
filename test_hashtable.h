#ifndef TEST_HASHTABLE_H
#define TEST_HASHTABLE_H

#include "DifferentStructures/HashTable.h"
#include <iostream>
#include <string>
#include <random>

void print_table(const HashTable<int, std::string>& table) {
    auto iterator = table.GetIterator();
    std::cout << "=== Current HashTable Contents ===\n";
    while (iterator->MoveNext()) {
        std::cout << "  [Key: " << iterator->GetCurrentKey()
                  << ", Value: " << iterator->GetCurrentValue() << "]\n";
    }
    std::cout << "==================================\n";
}

void basic_tests() {
    std::cout << "=== Running Basic HashTable Tests ===\n";

    HashTable<int, std::string> table;

    table.Add(101, "John");
    table.Add(202, "Emma");
    table.Add(303, "Liam");
    table.Add(404, "Sophia");
    print_table(table);

    if (table.ContainsKey(202)) {
        table.Update(202, "Emily");
        std::cout << "After updating key 202 (Emma -> Emily):\n";
        print_table(table);
    } else {
        std::cout << "Key 202 not found for update.\n";
    }

    if (table.ContainsKey(303)) {
        table.Remove(303);
        std::cout << "After removing key 303 (Liam):\n";
        print_table(table);
    } else {
        std::cout << "Key 303 not found for removal.\n";
    }

    try {
        std::cout << "Value at key 404: " << table.Get(404) << " (Sophia)\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    std::cout << "Contains key 101: " << (table.ContainsKey(101) ? "Yes" : "No") << "\n";
    std::cout << "Contains key 505: " << (table.ContainsKey(505) ? "Yes" : "No") << "\n";

    std::cout << "=== Basic HashTable Tests Completed ===\n";
}

void performance_testss() {
    std::cout << "=== Running Performance HashTable Tests ===\n";

    const int NUM_ELEMENTS = 5000;
    HashTable<int, std::string> table;

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> key_dist(1, NUM_ELEMENTS * 5);
    std::uniform_int_distribution<> value_dist(0, 25);

    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        int key = key_dist(gen);
        std::string value(1, alphabet[value_dist(gen)]);
        table.Add(key, value);
    }
    std::cout << "Inserted " << NUM_ELEMENTS << " elements into the HashTable.\n";

    for (int i = 0; i < NUM_ELEMENTS / 2; ++i) {
        int key = key_dist(gen);
        if (table.ContainsKey(key)) {
            table.Update(key, "Updated");
        }
    }
    std::cout << "Updated approximately " << NUM_ELEMENTS / 2 << " elements.\n";

    for (int i = 0; i < NUM_ELEMENTS / 3; ++i) {
        int key = key_dist(gen);
        if (table.ContainsKey(key)) {
            table.Remove(key);
        }
    }
    std::cout << "Deleted approximately " << NUM_ELEMENTS / 3 << " elements.\n";

    std::cout << "Final HashTable size after deletions: " << table.GetCount() << "\n";

    std::cout << "=== Performance HashTable Tests Completed ===\n";
}

// Тесты обработки коллизий
void collision_tests() {
    std::cout << "=== Running Collision HashTable Tests ===\n";

    HashTable<int, std::string> table;

    // Добавляем элементы, которые будут попадать в одну и ту же ячейку (настраиваем вручную)
    table.Add(10, "Alpha");
    table.Add(26, "Bravo"); // Предполагается, что 10 и 26 попадают в один бакет
    table.Add(42, "Charlie"); // Аналогично, 10 и 42 могут коллидировать

    // Проверка наличия элементов
    std::cout << "Initial collision test state:\n";
    print_table(table);

    // Удаление одного из коллизирующих элементов с проверкой
    if (table.ContainsKey(26)) {
        table.Remove(26);
        std::cout << "After removing key 26 (Bravo):\n";
        print_table(table);
    } else {
        std::cout << "Key 26 not found for removal.\n";
    }

    // Проверка доступности остальных элементов
    try {
        std::cout << "Value at key 10: " << table.Get(10) << " (Alpha)\n";
        std::cout << "Value at key 42: " << table.Get(42) << " (Charlie)\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    std::cout << "=== Collision HashTable Tests Completed ===\n";
}

// Обобщённый тестовый интерфейс
void test_hashtable() {
    std::cout << "=== Starting HashTable Tests ===\n";

    basic_tests();
    performance_testss();
    collision_tests();

    std::cout << "=== All HashTable Tests Completed ===\n";
}

#endif // TEST_HASHTABLE_H

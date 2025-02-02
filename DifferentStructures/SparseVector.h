#ifndef SPARSEVECTOR_H
#define SPARSEVECTOR_H

#include "IDictionary.h"
#include "ShrdPtr.h"
#include "DynamicArraySmart.h"
#include "KeyValue.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <functional>

template <typename TElement>
class SparseVector {
public:
    SparseVector(int length, UnqPtr<IDictionary<int, TElement>> dictionary)
            : length(length), elements(std::move(dictionary)) {}

    ~SparseVector() {}

    int GetLength() const {
        return length;
    }

    TElement GetElement(int index) const {
        if (index < 0 || index >= length) {
            throw std::out_of_range("Index is out of bounds.");
        }
        if (!elements) {
            throw std::runtime_error("Dictionary is not initialized.");
        }
        if (elements->ContainsKey(index)) {
            return elements->Get(index);
        } else {
            return TElement();
        }
    }

    void SetElement(int index, const TElement& value) {
        if (index < 0 || index >= length) {
            throw std::out_of_range("Index is out of bounds.");
        }
        if (value != TElement()) {
            elements->Add(index, value);
        } else {
            RemoveElement(index);
        }
    }

    void RemoveElement(int index) {
        if (index < 0 || index >= length) {
            throw std::out_of_range("Index is out of bounds.");
        }
        if (elements->ContainsKey(index)) {
            elements->Remove(index);
        }
    }

    void ForEach(void (*func)(int, const TElement&)) const {
        auto iterator = elements->GetIterator();
        while (iterator->MoveNext()) {
            int key = iterator->GetCurrentKey();
            TElement value = iterator->GetCurrentValue();
            func(key, value);
        }
    }

    void Map(std::function<TElement(TElement)> func) {
        DynamicArraySmart<KeyValue<int, TElement>> updates;
        auto iterator = elements->GetIterator();
        while (iterator->MoveNext()) {
            int key = iterator->GetCurrentKey();
            TElement value = iterator->GetCurrentValue();
            TElement newValue = func(value);
            updates.Append(KeyValue<int, TElement>(key, newValue));
        }
        for (int i = 0; i < updates.GetLength(); ++i) {
            const KeyValue<int, TElement>& kv = updates.Get(i);
            (*elements)[kv.key] = kv.value;
        }
    }

    void MultiplyByScalar(TElement scalar) {
        if (scalar == 0) {
            auto iterator = elements->GetIterator();
            while (iterator->MoveNext()) {
                elements->Remove(iterator->GetCurrentKey());
            }
        } else {
            Map([scalar](TElement x) { return x * scalar; });
        }
    }

    TElement Reduce(TElement (*func)(TElement, TElement), TElement initial) const {
        TElement result = initial;
        auto iterator = elements->GetIterator();
        while (iterator->MoveNext()) {
            TElement value = iterator->GetCurrentValue();
            result = func(result, value);
        }
        return result;
    }

    UnqPtr<IDictionaryIterator<int, TElement>> GetIterator() const {
        return elements->GetIterator();
    }

private:
    int length;
    UnqPtr<IDictionary<int, TElement>> elements;
};

#endif // SPARSEVECTOR_H

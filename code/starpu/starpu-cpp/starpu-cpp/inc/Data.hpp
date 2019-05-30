#pragma once

#include <memory>
#include <utility>

#include <starpu.h>

namespace starpu {

template <typename T>
class Data {
protected:
    std::shared_ptr<T> data;

public:
    // Copies the given type T data.
    Data(const T& copy_data)
        : data(copy_data)
    {
    }

    // Moves the given type T data.
    Data(T&& expiring_data)
        : data(std::move(expiring_data))
    {
    }

    Data(const Data&);

    Data(Data&&);

    // If you're initializing a Data class you must give some data!
    Data() = delete;

    ~Data();
};
} // namespace starpu

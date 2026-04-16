#include <concepts>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

using namespace std;

namespace core_numeric {

template <typename C>
concept Iterable = requires(C c) {
    begin(c);
    end(c);
    typename C::value_type;
};

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> same_as<T>;
};

template <typename T>
concept Divisible = requires(T a, size_t n) {
    { a / n } -> convertible_to<T>;
};

template <typename T>
concept LessThanComparable = requires(T a, T b) {
    { a < b } -> convertible_to<bool>;
};

template <typename T>
concept ArithmeticLike = Addable<T> && Divisible<T> && LessThanComparable<T> && requires(T a, T b) {
    { a - b } -> same_as<T>;
    { a * b } -> same_as<T>;
};

template <Iterable C>
requires Addable<typename C::value_type>
auto sum(const C& container) {
    using T = typename C::value_type;
    T result{};
    for (const auto& value : container) {
        result = result + value;
    }
    return result;
}

template <Iterable C>
requires Addable<typename C::value_type> && Divisible<typename C::value_type>
auto mean(const C& container) {
    using T = typename C::value_type;
    size_t count = 0;
    for (const auto& value : container) {
        (void)value;
        ++count;
    }
    if (count == 0) {
        throw invalid_argument("mean: el contenedor no puede estar vacio");
    }
    T total = sum(container);
    if constexpr (is_integral_v<T>) {
        return total / count;
    } else {
        return total / count;
    }
}

template <Iterable C>
requires ArithmeticLike<typename C::value_type>
auto variance(const C& container) {
    using T = typename C::value_type;
    size_t count = 0;
    for (const auto& value : container) {
        (void)value;
        ++count;
    }
    if (count == 0) {
        throw invalid_argument("variance: el contenedor no puede estar vacio");
    }
    T avg = mean(container);
    T acc{};
    for (const auto& value : container) {
        T diff = value - avg;
        acc = acc + (diff * diff);
    }
    return acc / count;
}

template <Iterable C>
requires LessThanComparable<typename C::value_type>
auto max(const C& container) {
    using T = typename C::value_type;
    auto it = begin(container);
    auto last = end(container);
    if (it == last) {
        throw invalid_argument("max: el contenedor no puede estar vacio");
    }
    T best = *it;
    ++it;
    for (; it != last; ++it) {
        if (best < *it) {
            best = *it;
        }
    }
    return best;
}

template <Iterable C, typename Func>
requires requires(typename C::value_type x, Func f) {
    { f(x) };
}
auto transform_reduce(const C& container, Func func) {
    auto it = begin(container);
    if (it == end(container)) {
        throw invalid_argument("transform_reduce: el contenedor no puede estar vacio");
    }
    using R = decltype(func(*it));
    static_assert(Addable<R>, "El resultado de la transformacion debe ser sumable");
    R result{};
    for (const auto& value : container) {
        result = result + func(value);
    }
    return result;
}

template <typename T, typename... Ts>
requires (Addable<T> && ... && same_as<T, Ts>)
auto sum_variadic(T first, Ts... rest) {
    return (first + ... + rest);
}

template <typename T, typename... Ts>
requires Addable<T> && Divisible<T> && (... && same_as<T, Ts>)
auto mean_variadic(T first, Ts... rest) {
    T total = sum_variadic(first, rest...);
    constexpr size_t n = 1 + sizeof...(Ts);
    if constexpr (is_integral_v<T>) {
        return total / n;
    } else {
        return total / n;
    }
}

template <typename T, typename... Ts>
requires (ArithmeticLike<T> && ... && same_as<T, Ts>)
auto variance_variadic(T first, Ts... rest) {
    constexpr size_t n = 1 + sizeof...(Ts);
    T avg = mean_variadic(first, rest...);
    T acc = (T{} + ... + ((rest - avg) * (rest - avg)));
    acc = acc + ((first - avg) * (first - avg));
    return acc / n;
}

template <typename T, typename... Ts>
requires (LessThanComparable<T> && ... && same_as<T, Ts>)
auto max_variadic(T first, Ts... rest) {
    T best = first;
    ((best = (best < rest ? rest : best)), ...);
    return best;
}

}

class Coordenada {
private:
    double x;
    double y;

public:
    Coordenada(double x_ = 0.0, double y_ = 0.0) : x(x_), y(y_) {}

    double norma2() const {
        return x * x + y * y;
    }

    Coordenada operator+(const Coordenada& other) const {
        return Coordenada(x + other.x, y + other.y);
    }

    Coordenada operator-(const Coordenada& other) const {
        return Coordenada(x - other.x, y - other.y);
    }

    Coordenada operator*(const Coordenada& other) const {
        return Coordenada(x * other.x, y * other.y);
    }

    Coordenada operator/(size_t n) const {
        return Coordenada(x / n, y / n);
    }

    bool operator<(const Coordenada& other) const {
        return norma2() < other.norma2();
    }

    friend ostream& operator<<(ostream& os, const Coordenada& c) {
        os << "(" << c.x << ", " << c.y << ")";
        return os;
    }
};

int main() {
    using namespace core_numeric;

    vector<int> vi{1, 2, 3, 4};
    vector<double> vd{1.0, 2.0, 3.0, 4.0};
    vector<Coordenada> puntos{{1.0, 2.0}, {3.0, 1.0}, {0.5, 0.5}};

    cout << "sum(vi) = " << sum(vi) << '\n';
    cout << "mean(vi) = " << mean(vi) << '\n';
    cout << "variance(vi) = " << variance(vi) << '\n';
    cout << "max(vi) = " << max(vi) << '\n';
    cout << "transform_reduce(vd, x*x) = "
         << transform_reduce(vd, []<typename U>(U x) { return x * x; }) << '\n';

    cout << "sum_variadic(1, 2, 33, 4) = " << sum_variadic(1, 2, 33, 4) << '\n';
    cout << "mean_variadic(1.0, 2.0, 3.0, 4.0) = " << mean_variadic(1.0, 2.0, 3.0, 4.0) << '\n';
    cout << "variance_variadic(1.0, 2.0, 3.0, 4.0) = " << variance_variadic(1.0, 2.0, 3.0, 4.0) << '\n';
    cout << "max_variadic(1.0, 2.7, 3.0, 4.0) = " << max_variadic(1.0, 2.7, 3.0, 4.0) << '\n';

    cout << "sum(puntos) = " << sum(puntos) << '\n';
    cout << "mean(puntos) = " << mean(puntos) << '\n';
    cout << "variance(puntos) = " << variance(puntos) << '\n';
    cout << "max(puntos) = " << max(puntos) << '\n';

}

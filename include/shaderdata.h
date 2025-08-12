#pragma once

#include <string>
#include <utility>
#include <type_traits>
#include <GL/gl.h> // oder <OpenGL/gl3.h> je nach Plattform

// =================================================================================================

struct UniformID
{
    GLint   m_location{};
    String  m_name{ "" };

    UniformID() = default;
    UniformID(String name, GLint location)
        : m_location{ location }, m_name{ std::move(name) }
    {
    }

    virtual ~UniformID() = default; // falls du polymorph speichern willst

    inline GLint& Location(void) { return m_location; }

    inline String& Name(void) { return m_name; }

    bool operator<(const UniformID& other) const noexcept { return m_name < other.m_name; }

    bool operator>(const UniformID& other) const noexcept { return m_name > other.m_name; }

    bool operator==(const UniformID& other) const noexcept { return m_name == other.m_name; }

    bool operator!=(const UniformID& other) const noexcept { return m_name != other.m_name; }
};

// -------------------------------------------------------------------------------------------------

template<typename T>
struct UniformData
    : public UniformID
{
    T m_value{};

    UniformData() = default;
    UniformData(String name, GLint location, T value = {})
        : UniformID(std::move(name), location), m_value(std::move(value)) {
    }

    // Zuweisung AUS einem T
    UniformData& operator=(const T& other) {
        m_value = other;
        return *this;
    }
    UniformData& operator=(T&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        m_value = std::move(other);
        return *this;
    }

    // optional: Vergleich mit T
    inline bool operator==(const T& other) const noexcept { return m_value == other; }
    inline bool operator!=(const T& other) const noexcept { return !(*this == other); }

    inline T& Value(void) { return m_value; }
};

// -------------------------------------------------------------------------------------------------

template<typename PointerType>
struct UniformArrayData<PointerType> {
    PointerType m_data;
    size_t      m_size;

    void operator()(PointerType data, size_t size) {
        return Create(value, size);
    }
};

template<typename PointerType>
struct UniformArray : public UniformID {
    static_assert(std::is_pointer_v<PointerType>, "UniformArray only works with pointer types.");
    using BaseType = std::remove_pointer_t<PointerType>;

    std::unique_ptr<BaseType[]> m_value; // eigener Speicher
    size_t                      m_size{ 0 }; // Größe in BYTES
    size_t                      m_length{ 0 };

    UniformArray() = default;

    bool Create(PointerType value, size_t length)
        if (not value or (length == 0)) {
            m_value.reset();
            m_size =
                m_length = 0;
            return true;
        }

    if (not m_value or (m_length != length)) {
        m_value = std::make_unique<BaseType[]>(m_length);
        m_size = m_length * sizeof(BaseType);
    }

    std::memcpy(m_value.get(), value, m_size);
    return true;
}

UniformArray(String name, GLint location, PointerType value = nullptr, size_t length = 0)
    : UniformID(std::move(name), location)
{
    Create(value, length);
}

// Setzen/Updaten via Funktionsaufruf-Operator:
// - Wenn Größe unterschiedlich: reallozieren + kopieren -> true zurück
// - Wenn Größe gleich: memcmp, falls anders -> kopieren -> true, sonst false
bool operator()(PointerType value, size_t length) {
    return Create(value, length);
}

bool operator==(PointerType other) const noexcept {
    return m_value and other and std::memcmp(m_value.get(), other, m_size) == 0;
}


// Vergleich mit externem Buffer + Bytegröße
bool operator==(PointerType other, size_t length) const noexcept {
    if (not m_value or not other)
        return false;
    if (m_length != length) // also catches size == 0
        return false;
    return std::memcmp(m_value.get(), other, m_size) == 0;
}


bool operator!=(PointerType other, size_t size) const noexcept {
    return !(*this == other, size); // Komma-Operator als Guard gegen versehentliche Klammerfehler
}

// Zugriff
inline PointerType Value() noexcept { return m_value.get(); }
inline const BaseType* Value() const noexcept { return m_value.get(); }

// Größen-API
static constexpr size_t Length() noexcept { return m_length; }
static constexpr size_t Size()    noexcept { return m_size; }
};

// -------------------------------------------------------------------------------------------------

template<typename PointerType, size_t ElemCount>
struct FixedUniformArray : public UniformArray<PointerType> {
    using Base = UniformArray<PointerType>;
    using BaseType = typename Base::BaseType;

    FixedUniformArray() = default;
    FixedUniformArray(String name, GLint location)
        : Base(std::move(name), location) {
    }

    // fester Create/Update ohne size-Parameter
    inline bool Create(PointerType values) {
        return Base::Create(values, ElemCount);
    }
    inline bool operator()(PointerType values) {
        return Base::Create(values, ElemCount);
    }

    // Vergleich gegen externen Buffer gleicher fester Größe
    inline bool operator==(PointerType other) const noexcept {
        return Base::operator==(other, ElemCount);
    }
    inline bool operator!=(PointerType other) const noexcept {
        return !(*this == other);
    }
};

// -------------------------------------------------------------------------------------------------

using UniformVector4f = UniformData<Vector4f>;

using UniformVector3f = UniformData<Vector3f>;

using UniformVector2f = UniformData<Vector2f>;

using UniformFloat = UniformData<Float>;

using UniformInt = UniformData<Int>;

using UniformArray9f = FixedUniformArray<float*, 9>;

using UniformArray16f = FixedUniformArray<float*, 16>;

using UniformArray2i = FixedUniformArray<int*, 2>;

using UniformArray3i = FixedUniformArray<int*, 3>;

using UniformArray4i = FixedUniformArray<int*, 4>;

// =================================================================================================


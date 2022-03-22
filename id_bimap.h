#ifndef IDBIMAP_H
#define IDBIMAP_H

#include <map>
#include <string>
#include <type_traits>

struct NoValueType
{};

template <typename mappedType = NoValueType, typename keyType = std::size_t>
class id_bimap
{
    public:
        using mapped_type = mappedType;
        using key_type = keyType;

        id_bimap()
        {
            static_assert(!std::is_same<mapped_type, NoValueType>::value,
                "Template parameter \"value\" must always be specified.");
            static_assert(!std::is_same<std::remove_const<std::remove_reference<mapped_type>>,
                std::remove_const<std::remove_reference<key_type>>>::value, "Key and value must be separate types.");
            static_assert(std::is_integral<key_type>::value, "Key must be integer!");
        }

        id_bimap(const id_bimap& p_other)
        { m_map = p_other.m_map; }

        id_bimap(id_bimap&& p_other) noexcept
            : m_map(std::move(p_other.m_map))
        {}

        ~id_bimap() = default;

        id_bimap& operator=(const id_bimap& p_other)
        {
            if (this != &p_other)
                return *this = id_bimap(p_other);
            return *this;
        }

        id_bimap& operator=(id_bimap&& p_other) noexcept
        {
            if (this != &p_other)
                m_map = std::move(p_other.m_map);
            return *this;
        }

    private:
        std::map<key_type, mapped_type> m_map;
};

template <typename mapped_type = NoValueType>
using kchar_id_bimap = id_bimap<mapped_type, char>;

using string_id_bimap = id_bimap<std::string>;

#endif
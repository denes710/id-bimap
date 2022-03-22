#ifndef IDBIMAP_H
#define IDBIMAP_H

#include <cassert>
#include <algorithm>
#include <map>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <memory>
#include <functional>
#include <set>

struct NoValueType
{};

template <typename mappedType = NoValueType, typename keyType = std::size_t>
class id_bimap
{
    private:
        struct MappedLess;
    public:
        using mapped_type = mappedType;
        using key_type = keyType;
        using TKeyMap = std::map<keyType, mappedType>;
        using TMappedMap = std::map<const std::reference_wrapper<const mapped_type>, key_type, MappedLess>;

        id_bimap()
        {
            static_assert(!std::is_same<mapped_type, NoValueType>::value,
                "Template parameter \"value\" must always be specified.");
            static_assert(!std::is_same<std::remove_const<std::remove_reference<mapped_type>>,
                std::remove_const<std::remove_reference<key_type>>>::value, "Key and value must be separate types.");
            static_assert(std::is_integral<key_type>::value, "Key must be integer!");
        }

        id_bimap(const std::initializer_list<mappedType>& p_values)
        {
            for(const auto& value : p_values)
                insert(value);
        }

        id_bimap(const id_bimap& p_other)
        {
            for (const auto& [key, value] : p_other.m_keyMap)
                insert(value);
        }

        id_bimap(id_bimap&& p_other) noexcept
            : m_keyMap(std::move(p_other.m_keyMap))
            , m_valuesMap(std::move(p_other.m_valuesMap))
            , m_currentIndex(p_other.m_currentIndex)
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
            {
                m_keyMap = std::move(p_other.m_keyMap);
                m_valuesMap = std::move(p_other.m_valuesMap);
                m_currentIndex = p_other.m_currentIndex;
            }
            return *this;
        }

        std::size_t size() const
        { return m_keyMap.size(); }

        bool empty() const
        { return m_keyMap.empty(); }

        void clear()
        {
            m_valuesMap.clear();
            m_keyMap.clear();
            m_currentIndex = 0;
        }

        std::pair<typename TKeyMap::const_iterator, bool> insert(const mappedType& p_value)
        {
            const auto it = std::find_if(
                m_keyMap.begin(),
                m_keyMap.end(),
                [&](const typename TKeyMap::value_type& p_elem){ return p_elem.second == p_value; });
            
            if (it != m_keyMap.end())
                return {it, false};
            
            const auto result = m_keyMap.insert({m_currentIndex++, p_value});
            m_valuesMap[result.first->second] = result.first->first;

            return result;
        }

        const key_type& operator[](const mapped_type& p_value) const
        {
            try
            {
                return m_valuesMap.at(p_value);
            }
            catch(const std::out_of_range& p_exception)
            {
                throw std::domain_error("domain error");
            }
        }

        const mapped_type& operator[](const key_type& p_value) const
        { return m_keyMap.at(p_value); }

        void erase(key_type p_value)
        {
            if (p_value >= m_currentIndex)
                return;

            const auto erased = m_keyMap.extract(p_value);
            if (erased.key() + 1 == m_currentIndex) // it was the last elem
            {
                --m_currentIndex;
                return;
            }

            for (auto index = erased.key() + 1; index < m_currentIndex; ++index)
            {
                auto elem = m_keyMap.extract(index);
                --elem.key();
                const auto key = elem.key();
                m_keyMap.insert(std::move(elem));
                m_valuesMap[m_keyMap[key]] = key;
            }
            --m_currentIndex;
        }

        void erase(const mapped_type& p_value)
        {
            try
            {
                erase(m_valuesMap.at(p_value));           
            }
            catch(const std::out_of_range& p_exception)
            {
                return;
            }
        }

        typename TKeyMap::const_iterator find(const mapped_type& p_value) const
        {
            return std::find_if(
                m_keyMap.begin(),
                m_keyMap.end(),
                [&](const typename TKeyMap::value_type& p_elem){ return p_elem.second == p_value; });
        }

        typename TKeyMap::const_iterator begin() const
        { return m_keyMap.begin(); }

        typename TKeyMap::const_iterator end() const
        { return m_keyMap.end(); }

        template<class... Args>
        std::pair<typename TKeyMap::const_iterator, bool> emplace(Args&&... args)
        { return m_keyMap.emplace(m_currentIndex++, std::forward<Args>(args)...); }

        typename TKeyMap::const_iterator find_if(std::function<bool(const mappedType&)> p_function) const
        {
            for (auto it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
            {
                if (p_function(it->second))
                    return it;
            }

            return m_keyMap.end();
        }

        void delete_all(std::function<bool(const mappedType&)> p_function)
        {
            std::set<key_type> remainedKey;
            for (auto it = m_keyMap.begin(); it != m_keyMap.end();)
            {
                if (p_function(it->second))
                    m_keyMap.erase(it++);
                else
                {
                    remainedKey.insert(it->first);
                    ++it;
                }
            }

            if (remainedKey.empty())
                return;

            m_currentIndex = 0;
            m_valuesMap.clear();
            if (m_keyMap.empty())
                return;

            for (const auto key : remainedKey)
            {
                auto current = m_keyMap.extract(key);
                current.key() = m_currentIndex++;
                m_keyMap.insert(std::move(current));
            }
        }

    private:
        struct MappedLess
        {
            bool operator()(const mapped_type& p_lhs, const mapped_type& p_rhs) const 
            { return p_lhs < p_rhs; }
        };

        TKeyMap m_keyMap;
        TMappedMap m_valuesMap;
        key_t m_currentIndex = 0;
};

template <typename mapped_type = NoValueType>
using kchar_id_bimap = id_bimap<mapped_type, char>;

using string_id_bimap = id_bimap<std::string>;

#endif
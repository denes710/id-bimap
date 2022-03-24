#ifndef IDBIMAP_H
#define IDBIMAP_H

#include <cassert>
#include <algorithm>
#include <functional> 
#include <map>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <set>
#include <vector>
#include <optional>
#include <memory>

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
        using TMappedMap = std::map<const std::reference_wrapper<const mapped_type>, key_type, MappedLess>;
        using TVector = std::vector<std::optional<mapped_type>>;

        struct Iterator
        {
            using iterator_category = std::input_iterator_tag;
            using value_type        = std::pair<const key_type&, const mapped_type&>;
            using reference_type    = const std::pair<const key_type&, const mapped_type&>&;
            using pointer_type      = const std::pair<const key_type&, const mapped_type&>*;

            Iterator(const TVector& p_vector, const TMappedMap& p_mapped, key_type p_index)
                : m_vector(p_vector)
                , m_mapped(p_mapped)
            {
                m_iterator = p_vector.cbegin();
                std::advance(m_iterator, p_index);
                m_current = std::make_shared<value_type>(m_mapped.at(m_iterator->value()), m_iterator->value());
            }

            Iterator(
                const TVector& p_vector,
                const TMappedMap& p_mapped,
                const typename TVector::const_iterator& p_iterator)
                : m_iterator(p_iterator)
                , m_vector(p_vector)
                , m_mapped(p_mapped)
            {}

            reference_type operator*() const
            { return *m_current.get(); }

            pointer_type operator->()
            { return m_current.get(); }

            Iterator& operator++()
            {
                ++m_iterator;
                if (m_iterator == m_vector.end())
                    return *this;

                for (; m_iterator != m_vector.end(); ++m_iterator)
                {
                    if (m_iterator->has_value())
                    {
                        m_current =
                            std::make_shared<value_type>(m_mapped.at(m_iterator->value()), m_iterator->value());
                        return *this;
                    }
                }
                return *this;
            }

            friend bool operator== (const Iterator& a, const Iterator& b)
            { return a.m_iterator == b.m_iterator; };

            friend bool operator!= (const Iterator& a, const Iterator& b)
            { return a.m_iterator != b.m_iterator; };

        private:
            typename TVector::const_iterator m_iterator;
            const TVector& m_vector;
            const TMappedMap& m_mapped;
            std::shared_ptr<value_type> m_current;
        };

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
            : m_vector(p_other.m_vector)
            , m_logicalDeletedKeys(p_other.m_logicalDeletedKeys)
            , m_reserveSize(p_other.m_reserveSize)
        {

            for (auto i = 0u; i < m_vector.size(); ++i)
            {
                if (m_vector[i])
                    m_valuesMap[*m_vector[i]] = i;
            }
        }

        id_bimap(id_bimap&& p_other) noexcept
            : m_vector(std::move(p_other.m_vector))
            , m_valuesMap(std::move(p_other.m_valuesMap))
            , m_logicalDeletedKeys(std::move(p_other.m_logicalDeletedKeys))
            , m_reserveSize(p_other.m_reserveSize)
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
                m_vector = std::move(p_other.m_vector);
                m_valuesMap = std::move(p_other.m_valuesMap);
                m_logicalDeletedKeys = std::move(p_other.m_logicalDeletedKeys);
                m_reserveSize = p_other.m_reserveSize;
            }
            return *this;
        }

        std::size_t size() const
        { return m_valuesMap.size(); }

        bool empty() const
        { return m_valuesMap.empty(); }

        void clear()
        {
            m_valuesMap.clear();
            m_vector.clear();
            m_logicalDeletedKeys.clear();
            m_reserveSize = 0;
        }

        std::pair<Iterator, bool> insert(const mappedType& p_value)
        {
            const auto it = find(p_value);

            if (it != end())
                return {it, false};

            const auto index = pop_next_index();
            if (index < m_vector.size())
            {
                const auto inputIndex = index;
                m_vector[index].emplace(p_value);
                m_valuesMap[*m_vector[index]] = index;
            }
            else
            {
                const auto prevCapacity = m_vector.capacity();
                m_vector.push_back(p_value);

                if (prevCapacity == m_vector.capacity())
                {
                    if (m_reserveSize)
                        --m_reserveSize;

                    m_valuesMap[*m_vector[index]] = index;
                }
                else
                {
                    m_reserveSize = 0;
                    UpdateValueMap();
                }
            }

            return {Iterator(m_vector, m_valuesMap, index), true};
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

        const mapped_type& operator[](const key_type& p_key) const
        {
            if (p_key < m_vector.size())
            {
                if (m_vector[p_key].has_value())
                    return *m_vector[p_key];
            }
            throw std::out_of_range("out of range");
        }

        void erase(key_type p_key)
        {
            if (p_key < m_vector.size())
            {
                if (m_vector[p_key].has_value())
                {
                    m_valuesMap.erase(*m_vector[p_key]);
                    m_logicalDeletedKeys.insert(p_key);
                    m_vector[p_key].reset();
                }
            }
        }

        void erase(const mapped_type& p_value)
        {
            const auto it = m_valuesMap.find(p_value);

            if (it == m_valuesMap.end())
                return;

            const auto key = it->second;
            m_valuesMap.erase(it);
            m_logicalDeletedKeys.insert(key);
            m_vector[key].reset();
        }

        Iterator find(const mapped_type& p_value) const
        {
            try
            {
                const auto key = m_valuesMap.at(p_value);
                return Iterator(m_vector, m_valuesMap, key);
            }
            catch(const std::out_of_range& p_exception)
            {
                return end();
            }
        }

        Iterator begin() const
        {
            for (auto i = 0u; i < m_vector.size(); ++i)
            {
                if (m_vector[i].has_value())
                    return Iterator(m_vector, m_valuesMap, i);
            }

            return end();
        }

        Iterator end() const
        { return Iterator(m_vector, m_valuesMap, m_vector.end()); }

        template<class... Args>
        std::pair<Iterator, bool> emplace(Args&&... args)
        {
            const auto index = pop_next_index();
            if (index < m_vector.size())
            {
                m_vector[index].emplace(std::forward<Args>(args)...);
                m_valuesMap[*m_vector[index]] = index;
            }
            else
            {
                const auto prevCapacity = m_vector.capacity();

                m_vector.emplace_back(std::forward<Args>(args)...);

                if (prevCapacity == m_vector.capacity())
                {
                    if (m_reserveSize)
                        --m_reserveSize;

                    m_valuesMap[*m_vector[index]] = index;
                }
                else
                {
                    m_reserveSize = 0;
                    UpdateValueMap();
                }
            }

            return {Iterator(m_vector, m_valuesMap, index), true};
        }

        Iterator find_if(std::function<bool(const mappedType&)> p_function) const
        {
            for (auto i = 0u; i != m_vector.size(); ++i)
            {
                if (m_vector[i].has_value() && p_function(*m_vector[i]))
                    return Iterator(m_vector, m_valuesMap, i);
            }

            return end();
        }

        void delete_all(std::function<bool(const mappedType&)> p_function)
        {
            for (auto i = 0u; i != m_vector.size(); ++i)
            {
                if (m_vector[i].has_value() && p_function(*m_vector[i]))
                {
                    m_logicalDeletedKeys.insert(i);
                    m_valuesMap.erase(*m_vector[i]);
                    m_vector[i].reset();
                }
            }
        }

        key_type next_index() const
        { return m_logicalDeletedKeys.empty() ? m_vector.size() : *m_logicalDeletedKeys.begin(); }

        std::size_t capacity() const
        { return m_vector.size() + m_reserveSize; }

        bool is_contiguous() const
        {
            bool hasValue = false;
            for (auto it = m_vector.rbegin(); it != m_vector.rend(); ++it)
            {
                if (it->has_value())
                    hasValue = true;
                else if (hasValue)
                    return false;
            }

            return true;
        }

        void reserve(std::size_t p_size)
        {
            if (p_size > m_vector.size())
            {
                m_reserveSize = p_size - m_vector.size();
                const auto prevCapacity = m_vector.capacity();
                m_vector.reserve(p_size);
                if (prevCapacity < p_size)
                    UpdateValueMap();
            }
            else if (p_size  < m_vector.size())
            {
                if (m_logicalDeletedKeys.empty())
                    return;


                auto count = 0u;
                for (auto it = m_vector.rbegin(); it != m_vector.rend(); ++it)
                {
                    if (it->has_value())
                        break;
                    ++count;
                }

                const auto numberOfDeletion = m_vector.size() - p_size;

                if (numberOfDeletion > count)
                    return;

                for (auto i = 0; i < numberOfDeletion; ++i)
                    m_logicalDeletedKeys.erase(prev(m_logicalDeletedKeys.end()));

                m_reserveSize = 0;
                const auto prevCapacity = m_vector.capacity();
                m_vector.reserve(p_size);
                if (prevCapacity < p_size)
                    UpdateValueMap();
            }
            else
            {
                m_reserveSize = 0;
            }
        }

    private:
        struct MappedLess
        {
            bool operator()(const mapped_type& p_lhs, const mapped_type& p_rhs) const 
            { return p_lhs < p_rhs; }
        };

        key_type pop_next_index()
        {
            if (m_logicalDeletedKeys.empty())
                return m_vector.size();

            const auto ret = *m_logicalDeletedKeys.begin();
            m_logicalDeletedKeys.erase(m_logicalDeletedKeys.begin());
            return ret;
        }

        void UpdateValueMap()
        {
            m_valuesMap.clear();
            for (auto i = 0; i < m_vector.size(); ++i)
            {
                if (m_vector[i].has_value())
                    m_valuesMap[*m_vector[i]] = i;
            }
        }

        TVector m_vector;
        TMappedMap m_valuesMap;
        std::set<key_type> m_logicalDeletedKeys;
        unsigned m_reserveSize = 0;
};

template <typename mapped_type = NoValueType>
using kchar_id_bimap = id_bimap<mapped_type, char>;

using string_id_bimap = id_bimap<std::string>;

#endif
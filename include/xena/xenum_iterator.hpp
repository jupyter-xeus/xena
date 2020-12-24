/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XENA_ENUM_ITERATOR_HPP
#define XENA_ENUM_ITERATOR_HPP

#include <cstddef>
#include <string>

#include "xtl/xiterator_base.hpp"

namespace xena
{
    //TODO : move this file to XTL

    template <class E>
    class xenum_iterator : public xtl::xbidirectional_iterator_base<xenum_iterator<E>,
                                                                    E,
                                                                    std::ptrdiff_t,
                                                                    E*,
                                                                    E>
    {
    public:

        using self_type = xenum_iterator<E>;
        using value_type = E;
        using reference = E;
        using pointer = E*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        xenum_iterator() = default;
        explicit xenum_iterator(value_type value);

        self_type& operator++();
        self_type& operator--();

        reference operator*() const;
        pointer operator->() const;

    private:

        using underlying_type = std::underlying_type_t<E>;
        value_type m_value;
    };

    template <class E>
    bool operator==(const xenum_iterator<E>& lhs,
                    const xenum_iterator<E>& rhs);

    template <class E>
    bool operator<(const xenum_iterator<E>& lhs,
                   const xenum_iterator<E>& rhs);

    template <class I>
    class xiterator_range
    {
    public:

        using iterator = I;

        xiterator_range(I first, I last);

        iterator begin() const;
        iterator end() const;

    private:

        iterator m_first;
        iterator m_last;
    };

    template <class I>
    xiterator_range<I> make_iterator_range(I first, I last);

    template <class E>
    xiterator_range<xenum_iterator<E>> make_enum_range();

    /******************
     * xenum_iterator *
     ******************/
    
    template <class E>
    inline xenum_iterator<E>::xenum_iterator(value_type value)
        : m_value(value)
    {
    }

    template <class E>
    inline auto xenum_iterator<E>::operator++() -> self_type&
    {
        m_value = value_type(underlying_type(m_value) + 1);
        return *this;
    }
    
    template <class E>
    inline auto xenum_iterator<E>::operator--() -> self_type&
    {
        m_value = value_type(underlying_type(m_value) - 1);
        return *this;
    }

    template <class E>
    inline auto xenum_iterator<E>::operator*() const -> reference
    {
        return m_value;
    }

    template <class E>
    inline auto xenum_iterator<E>::operator->() const -> pointer
    {
        return &m_value;
    }

    template <class E>
    inline bool operator==(const xenum_iterator<E>& lhs,
                           const xenum_iterator<E>& rhs)
    {
        return *lhs == *rhs;
    }

    template <class E>
    inline bool operator<(const xenum_iterator<E>& lhs,
                          const xenum_iterator<E>& rhs)
    {
        return *lhs < *rhs;
    }

    /*******************
     * xiterator_range *
     *******************/

    template <class I>
    inline xiterator_range<I>::xiterator_range(I first, I last)
        : m_first(first), m_last(last)
    {
    }

    template <class I>
    inline auto xiterator_range<I>::begin() const -> iterator
    {
        return m_first;
    }

    template <class I>
    inline auto xiterator_range<I>::end() const -> iterator
    {
        return m_last;
    }

    template <class I>
    inline xiterator_range<I> make_iterator_range(I first, I last)
    {
        return xiterator_range<I>(first, last);
    }

    template <class E>
    inline xiterator_range<xenum_iterator<E>> make_enum_range()
    {
        return make_iterator_range(xenum_iterator<E>(E::first),
                                   xenum_iterator<E>(E::last));
    }
}

#endif


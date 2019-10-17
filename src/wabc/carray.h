/*
* Copyright[yyyy][name of copyright owner]
* Copyright (c) 2019, Yuan zhi wei <Racer_y@126.com>. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http ://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include <assert.h>
#include <string>
#include <stdexcept>

#pragma warning( push )
#pragma warning( disable : 4996 )

// --------------------------------------------------------------------

template< size_t N, typename T >
struct carray
{
	enum{ length = N };

	T	m_data[N];

	size_t capacity()const throw() { return N; }
	size_t size()const throw() { return N; }

	T * begin() throw() { return m_data; }
	T * end() throw() { return m_data + N ; }

	const T * begin()const throw() { return m_data; }
	const T * end()const throw() { return m_data + N ; }

	T & operator[]( size_t n )
	{
		assert( n < N );
		return m_data[n];
	}

	const T & operator[]( size_t n )const
	{
		assert( n < N );
		return m_data[n];
	}

	T & at( size_t n )
	{
		if ( n >= N )
			throw std::range_error();

		return m_data[n];
	}

	const T & at( size_t n )const
	{
		if ( n >= N )
			throw std::range_error();
		return m_data[n];
	}

	carray< N, T > & operator=( const carray< N, T > & rhs )
	{
		if ( this != &rhs )
			std::copy( rhs.m_data, rhs.m_data + N, m_data );
		return *this;
	}

	carray< N, T > & assign(const T *rhs, size_t n)
	{
		if(n > N)
			n= N;
		std::copy( rhs, rhs + n, m_data );
		return *this;
	}
};

// --------------------------------------------------------------------

template< size_t N >
struct carray<N, char>
{
	typedef char E;

	typedef std::char_traits<E> traits_type;
	typedef const E & const_reference;
	typedef E & reference;
	typedef E char_type;

	typedef E * pointer;
	typedef const E * const_pointer;

	typedef carray< N, E > self;

	enum{ length= N };

	E	m_data[N];

	reference operator[]( size_t n )
	{
		assert( n < N );
		return m_data[n];
	}

	const_reference operator[]( size_t n )const
	{
		assert( n < N );
		return m_data[n];
	}

	pointer c_buffer() throw(){ return m_data; }

	const_pointer c_str()const throw(){ return m_data; }

	size_t size()const { return traits_type::length(m_data); }

	template< typename Tr, typename Alloc >
	self & operator=( const std::basic_string<E, Tr, Alloc> & rhs )
	{
		return assign( rhs.c_str(), rhs.size() );
	}

	template< size_t M >
		self & operator=( const carray<M, E> & rhs )
	{
		return assign( rhs.m_data, M );
	}

	self & operator=( const self & rhs )
	{
		if ( m_data != rhs.m_data )
			traits_type::copy( m_data, rhs.m_data, N );

		return *this;
	}

	self & operator=( const_pointer lpsz )
	{
		if ( m_data != lpsz )
			return assign(lpsz);
		return *this;
	}

	template< typename Tr, typename Alloc >
	self & operator+=( const std::basic_string<E, Tr, Alloc> & rhs )
	{
		return append( rhs.c_str(), rhs.size() );
	}

	template< size_t M >
		self & operator+=( const carray<M, E> & rhs )
	{
		return append( rhs.m_data, M );
	}

	self & operator+=( const self & rhs )
	{
		return append( rhs.m_data, N );
	}

	self & operator+=( const_pointer lpsz )
	{
		return append(lpsz);
	}

	self & assign( const_pointer lpsz, size_t nLen )
	{
		const size_t n= N > nLen ? nLen : (N - 1);
		m_data[n]= 0;
		traits_type::copy( m_data, lpsz, n );
		return *this;
	}

	self & assign( const_pointer lpsz )
	{
		return assign( lpsz, traits_type::length(lpsz) );
	}

	self & append( const_pointer lpsz, size_t nLen )
	{
		const size_t nn= size();
		const size_t n1= N - nn;
		const size_t n= n1 > nLen ? nLen : (n1 - 1);

		m_data[ nn + n ]= 0;
		traits_type::copy( m_data + nn, lpsz, n );
		return *this;
	}

	self & append( const_pointer lpsz )
	{
		return append( lpsz, traits_type::length(lpsz) );
	}

	self & insert(size_t pos, char_type ch)
	{
		const size_t len = traits_type::length(m_data);
		if ((len + 1) < N)
		{
			if (pos < len)
				::memmove(m_data + pos + 1, m_data + pos, (len - pos + 1) * sizeof(m_data[0]));
			else
				m_data[len + 1] = 0;
			m_data[pos] = ch;
		}
		return *this;
	}

	void erase(pointer first, pointer last)
	{
		if ( last >= (m_data+N) )
		{
			if ( first < (m_data+N) )
				*first= 0;
		}
		else if (first < last)
		{
			m_data[N-1]= 0;
			while(*last)
				*(first++)= *(last++);
			*first= 0;
		}
	}

	int compare( const_pointer lpsz )const throw()
	{
		const size_t n1= size() + 1;
		const size_t n2= traits_type::length(lpsz) + 1;
		return traits_type::compare( m_data, lpsz, n1 > n2 ? n2:n1 );
	}

	int compare( const self &rhs)const throw()
	{ return compare(rhs.m_data); }

	bool operator == ( const_pointer lpsz )const throw()
	{ return compare( lpsz ) == 0; }

	bool operator == ( const self & rhs )const throw()
	{ return compare( rhs.m_data ) == 0; }

	template< size_t M >
	bool operator == ( const carray<M,E> & rhs )const throw()
	{ return compare( rhs.m_data ) == 0; }

	bool operator != ( const_pointer lpsz )const throw()
	{ return compare( lpsz ) != 0; }

	bool operator != ( const self & rhs )const throw()
	{ return compare( rhs.m_data ) != 0; }

	template< size_t M >
	bool operator != ( const carray<M,E> & rhs )const throw()
	{ return compare( rhs.m_data ) != 0; }

	bool operator < ( const_pointer lpsz )const throw()
	{ return compare( lpsz ) < 0; }

	bool operator < ( const self & rhs )const throw()
	{ return compare( rhs.m_data ) < 0; }

	template< size_t M >
	bool operator < ( const carray<M,E> & rhs )const throw()
	{ return compare( rhs.m_data ) < 0; }

	template< typename Tr, typename Alloc >
	bool operator ==( const std::basic_string<E, Tr, Alloc> & rhs )
	{ return compare(rhs.c_str()) == 0; }

	template< typename Tr, typename Alloc >
	bool operator !=( const std::basic_string<E, Tr, Alloc> & rhs )
	{ return compare(rhs.c_str()) != 0; }

	template< typename Tr, typename Alloc >
	bool operator <( const std::basic_string<E, Tr, Alloc> & rhs )
	{ return compare(rhs.c_str()) < 0; }

	template< typename Tr, typename Alloc >
	bool operator >( const std::basic_string<E, Tr, Alloc> & rhs )
	{ return compare(rhs.c_str()) > 0; }
};

// --------------------------------------------------------------------

template< size_t N >
struct carray<N, wchar_t>
{
	typedef wchar_t E;

	typedef std::char_traits<E> traits_type;
	typedef const E & const_reference;
	typedef E & reference;
	typedef E char_type;

	typedef E * pointer;
	typedef const E * const_pointer;

	typedef carray< N, E > self;

	enum{ length = N };

	E	m_data[N];

	reference operator[](size_t n)
	{
		assert(n < N);
		return m_data[n];
	}

	const_reference operator[](size_t n)const
	{
		assert(n < N);
		return m_data[n];
	}

	pointer c_buffer() throw(){ return m_data; }

	const_pointer c_str()const throw(){ return m_data; }

	size_t size()const { return traits_type::length(m_data); }

	template< typename Tr, typename Alloc >
	self & operator=(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return assign(rhs.c_str(), rhs.size());
	}

	template< size_t M >
	self & operator=(const carray<M, E> & rhs)
	{
		return assign(rhs.m_data, M);
	}

	self & operator=(const self & rhs)
	{
		if (m_data != rhs.m_data)
			traits_type::copy(m_data, rhs.m_data, N);

		return *this;
	}

	self & operator=(const_pointer lpsz)
	{
		if (m_data != lpsz)
			return assign(lpsz);
		return *this;
	}

	template< typename Tr, typename Alloc >
	self & operator+=(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return append(rhs.c_str(), rhs.size());
	}

	template< size_t M >
	self & operator+=(const carray<M, E> & rhs)
	{
		return append(rhs.m_data, M);
	}

	self & operator+=(const self & rhs)
	{
		return append(rhs.m_data, N);
	}

	self & operator+=(const_pointer lpsz)
	{
		return append(lpsz);
	}

	self & assign(const_pointer lpsz, size_t nLen)
	{
		const size_t n = N > nLen ? nLen : (N - 1);
		m_data[n] = 0;
		traits_type::copy(m_data, lpsz, n);
		return *this;
	}

	self & assign(const_pointer lpsz)
	{
		return assign(lpsz, traits_type::length(lpsz));
	}

	self & append(const_pointer lpsz, size_t nLen)
	{
		const size_t nn = size();
		const size_t n1 = N - nn;
		const size_t n = n1 > nLen ? nLen : (n1 - 1);

		m_data[nn + n] = 0;
		traits_type::copy(m_data + nn, lpsz, n);
		return *this;
	}

	self & append(const_pointer lpsz)
	{
		return append(lpsz, traits_type::length(lpsz));
	}

	self & append(char_type ch)
	{
		const size_t len = traits_type::length(m_data);
		if ((len + 1) < N)
		{
			m_data[len] = ch;
			m_data[len + 1] = 0;
		}
		return *this;
	}

	void erase(pointer first, pointer last)
	{
		if (last >= (m_data + N))
		{
			if (first < (m_data + N))
				*first = 0;
		}
		else if (first < last)
		{
			m_data[N - 1] = 0;
			while (*last)
				*(first++) = *(last++);
			*first = 0;
		}
	}

	self & insert(size_t pos, char_type ch)
	{
		const size_t len = traits_type::length(m_data);
		if ((len + 1) < N)
		{
			if (pos < len)
				::memmove(m_data + pos + 1, m_data + pos, (len - pos + 1) * sizeof(m_data[0]));
			else
				m_data[len + 1] = 0;
			m_data[pos] = ch;
		}
		return *this;
	}

	int compare(const_pointer lpsz)const throw()
	{
		const size_t n1 = size() + 1;
		const size_t n2 = traits_type::length(lpsz) + 1;
		return traits_type::compare(m_data, lpsz, n1 > n2 ? n2 : n1);
	}

	int compare(const self &rhs)const throw()
	{
		return compare(rhs.m_data);
	}

	bool operator == (const_pointer lpsz)const throw()
	{
		return compare(lpsz) == 0;
	}

	bool operator == (const self & rhs)const throw()
	{
		return compare(rhs.m_data) == 0;
	}

	template< size_t M >
	bool operator == (const carray<M, E> & rhs)const throw()
	{
		return compare(rhs.m_data) == 0;
	}

	bool operator != (const_pointer lpsz)const throw()
	{
		return compare(lpsz) != 0;
	}

	bool operator != (const self & rhs)const throw()
	{
		return compare(rhs.m_data) != 0;
	}

	template< size_t M >
	bool operator != (const carray<M, E> & rhs)const throw()
	{
		return compare(rhs.m_data) != 0;
	}

	bool operator < (const_pointer lpsz)const throw()
	{
		return compare(lpsz) < 0;
	}

	bool operator < (const self & rhs)const throw()
	{
		return compare(rhs.m_data) < 0;
	}

	template< size_t M >
	bool operator < (const carray<M, E> & rhs)const throw()
	{
		return compare(rhs.m_data) < 0;
	}

	template< typename Tr, typename Alloc >
	bool operator ==(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return compare(rhs.c_str()) == 0;
	}

	template< typename Tr, typename Alloc >
	bool operator !=(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return compare(rhs.c_str()) != 0;
	}

	template< typename Tr, typename Alloc >
	bool operator <(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return compare(rhs.c_str()) < 0;
	}

	template< typename Tr, typename Alloc >
	bool operator >(const std::basic_string<E, Tr, Alloc> & rhs)
	{
		return compare(rhs.c_str()) > 0;
	}
};

// --------------------------------------------------------------------

template< size_t N, typename T >
carray<N,T> & cast_array(T(&a)[N])
{
	typedef carray<N,T> type;
	return reinterpret_cast<type &>(a);
}


template< size_t N, typename T >
const carray<N,T> & cast_array(const T(&a)[N])
{
	typedef const carray<N,T> type;
	return reinterpret_cast<type &>(a);
}

template< size_t N, typename T >
carray<N,T> & cast_array(T(*a)[N])
{
	typedef carray<N,T> type;
	return reinterpret_cast<type &>(*a);
}

template< size_t N, typename T >
const carray<N,T> & cast_array(const T(*a)[N])
{
	typedef const carray<N,T> type;
	return reinterpret_cast<type &>(*a);
}

#define _ca(a) cast_array(a)

#pragma warning( pop ) 

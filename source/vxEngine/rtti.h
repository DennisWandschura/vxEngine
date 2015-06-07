#pragma once
/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vxLib\types.h>
#include <vxLib\util\CityHash.h>
#include <vxLib\Container\sorted_vector.h>
#include <vxLib\Singleton.h>
#include <string>

namespace rtti
{
	namespace detail
	{
		template<class T>
		inline void setDataImpl(void *p, const void *pSrc, u32 offset)
		{
			*(T*)((ptrdiff_t)p + offset) = *(T*)pSrc;
		}

		template<class T, typename std::enable_if<std::is_copy_assignable<T>::value>::type>
		inline void setDataImpl(void *p, const T &value, u32 offset)
		{
			*(T*)((ptrdiff_t)p + offset) = value;
		}

		template <typename TYPE>
		void construct(void *ptr)
		{
			// Use placement new to call the constructor
			new (ptr)TYPE{};
		}

		template <typename TYPE>
		void destroy(void *ptr)
		{
			// Explicit call of the destructor
			((TYPE*)ptr)->TYPE::~TYPE();
		}
	}

	struct TypeData;

	struct Member
	{
		std::string m_name;
		size_t m_offset;
		const TypeData *m_pTypeData;

		Member();
		Member(const char *name, size_t offset, const TypeData *pTypeData);
	};

	struct TypeData
	{
		typedef void(*ConstructObjectFunc)(void*);
		typedef void(*DestructObjectFunc)(void*);
		typedef void(*SetDataFun)(void*, const void*, u32);

		std::string m_name;
		size_t m_size;
		u64 m_sid;
		vx::sorted_vector<u64, Member> m_members;
		ConstructObjectFunc m_constructFun;
		DestructObjectFunc m_destructFun;
		u32 m_alignment;

		TypeData() :m_name(), m_size(0), m_sid(0), m_members(), m_constructFun(nullptr), m_destructFun(nullptr), m_alignment(0){}
		~TypeData(){}

		void registerMember(const char *name, size_t offset, const TypeData *pTypeData);

		const Member* getMember(const char *name) const;

		friend bool operator==(const TypeData &lhs, const TypeData &rhs)
		{
			return lhs.m_sid == rhs.m_sid;
		}

		friend bool operator!=(const TypeData &lhs, const TypeData &rhs)
		{
			return lhs.m_sid != rhs.m_sid;
		}
	};

	class TypeCreatorBase
	{
	public:
		virtual ~TypeCreatorBase(){}

		virtual void setData(void *pDest, const void *pSrc, size_t offset);
		virtual void setMemberData(void *pDest, const void *pSrc, const char *memberName);
	};

	template<class T>
	struct TypeCreator;

	class Manager
	{
		vx::sorted_vector<u64, const TypeData*>  m_data;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;

	public:
		Manager(){}
		~Manager(){}

		void addType(const TypeData *pData, TypeCreatorBase *pCreator);

		template<class U>
		const char* getTypeName()
		{
			return TypeCreator<U>::getName();
		}

		const TypeData* getTypeData(const char *type);

		template<class U>
		const TypeData* getTypeData()
		{
			return TypeCreator<U>::getTypeData();
		}

		void setData(const char *type, void *pDest, void *pSrc, u32 offset);

		template<class U>
		void setData(const char *type, void *pDest, U value, u32 offset)
		{
			setData(type, pDest, &value, offset);
		}

		void setMemberData(const char *type, const char *memberName, void *pDest, const void *pSrc);

		template<class U>
		void setMemberData(const char *type, void *pDest, const char *memberName, U value)
		{
			setMemberData(type, memberName, pDest, &value);
		}

		template<class T, class U>
		void setMemberData(T *pDest, const char *memberName, U value)
		{
			TypeCreator<T>::setMemberData(pDest, memberName, value);
		}

		void destroy(const TypeData *pData, void *ptr);
	};

	typedef vx::GlobalSingleton<Manager, vx::AssertCheck, vx::CreationImplicit> SingletonRTTI;

	template<class T>
	struct TypeCreator : public TypeCreatorBase
	{
		static TypeData* getTypeDataPrivate()
		{
			static TypeData s_typeData;
			return &s_typeData;
		}

		static const Member* getMember(const char *memberName)
		{
			return getTypeData()->getMember(memberName);
		}

	public:
		TypeCreator(const char *name)
		{
			auto pTypeData = getTypeDataPrivate();

			pTypeData->m_name = name;
			pTypeData->m_size = sizeof(T);
			pTypeData->m_alignment = __alignof(T);
			pTypeData->m_sid = CITYHASH64(name);
			pTypeData->m_constructFun = (TypeData::ConstructObjectFunc)&detail::construct<T>;
			pTypeData->m_destructFun = (TypeData::DestructObjectFunc)&detail::destroy<T>;

			registerMembers();

			SingletonRTTI::get().addType(pTypeData, this);
		}

		T* nullcast()
		{
			return ((T*)0);
		}

		void addMember(const char *name, size_t offset,const TypeData *pTypeData)
		{
			getTypeDataPrivate()->registerMember(name, offset, pTypeData);
		}

		void registerMembers();

		/*void setData(void *pDest, const void *pSrc, size_t offset)
		{
			detail::setDataImpl<T>(pDest, *(const T*)pSrc, offset);
		}*/

		void setMemberData(void *pDest, const void *pSrc, const char *memberName)
		{
			auto p = getMember(memberName);
			if (p)
			{
				setData(pDest, pSrc, p->m_offset);
			}
		}

		static const TypeData* getTypeData()
		{
			return getTypeDataPrivate();
		}

		static const char* getName()
		{
			return getTypeData()->m_name.c_str();
		}

		template<class U>
		static void setMemberData(T *pDest, const char *memberName, U value)
		{
			auto p = getMember(memberName);
			if (p)
			{
				detail::setDataImpl<U>((void*)pDest, value, p->m_offset);
			}
		}
	};
}

#define _PASTE( _, __ )  _##__
#define _NAME_GENERATOR_INTERNAL( _ ) _PASTE( GENERATED_NAME, _ )
#define _NAME_GENERATOR( ) _NAME_GENERATOR_INTERNAL( __COUNTER__ )

#define RTTI_TYPE(TYPE) \
	rtti::TypeCreator<TYPE> _NAME_GENERATOR() {#TYPE};\
	template<> void rtti::TypeCreator<TYPE>::registerMembers()

#define RTTI_MEMBER(MEMBER) \
	addMember(#MEMBER, (size_t)(&nullcast()->MEMBER), rtti::SingletonRTTI::get().getTypeData<std::decay<decltype(nullcast()->MEMBER)>::type>() )
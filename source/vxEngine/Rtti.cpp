#include "rtti.h"

namespace rtti
{
	Member::Member()
		:m_name(),
		m_offset(0),
		m_pTypeData(nullptr)
	{
	}

	Member::Member(const char *name, size_t offset, const TypeData *pTypeData)
		:m_name(name),
		m_offset(offset),
		m_pTypeData(pTypeData)
	{
	}

	void TypeData::registerMember(const char *name, size_t offset, const TypeData *pTypeData)
	{
		U64 sid = pTypeData->m_sid;
		auto it = m_members.find(sid);
		if (it == m_members.end())
		{
			m_members.insert(sid, Member(name, offset, pTypeData));
		}
	}

	const Member* TypeData::getMember(const char *name) const
	{
		U64 sid = CITYHASH64(name);
		auto it = m_members.find(sid);
		if (it == m_members.end())
			return nullptr;

		return &*it;
	}

	void Manager::addType(const TypeData *pData, TypeCreatorBase *pCreator)
	{
		auto sid = pData->m_sid;
		auto it = m_data.find(sid);
		if (it == m_data.end())
		{
			m_data.insert(sid, pData);

			//m_creators.insert(sid, pCreator);
		}
	}

	/*

	CreatorBase

	*/

	void TypeCreatorBase::setData(void *pDest, const void *pSrc, size_t offset)
	{
		VX_UNREFERENCED_PARAMETER(pDest);
		VX_UNREFERENCED_PARAMETER(pSrc);
		VX_UNREFERENCED_PARAMETER(offset);
	}

	void TypeCreatorBase::setMemberData(void *pDest, const void *pSrc, const char *memberName)
	{
		VX_UNREFERENCED_PARAMETER(pDest);
		VX_UNREFERENCED_PARAMETER(pSrc);
		VX_UNREFERENCED_PARAMETER(memberName);
	}

	/*
	
	Manager

	*/

	const TypeData* Manager::getTypeData(const char *type)
	{
		U64 sid = CITYHASH64(type);
		auto it = m_data.find(sid);
		if (it == m_data.end())
			return nullptr;

		return *it;
	}

	void Manager::setData(const char *type, void *pDest, void *pSrc, U32 offset)
	{
		/*U64 sid = CITYHASH64(type);
		auto it = m_creators.find(sid);
		if (it == m_creators.end())
			return;

		(*it)->second->setData(pDest, pSrc, offset);*/
	}

	void Manager::setMemberData(const char *type, const char *memberName, void *pDest, const void *pSrc)
	{
	/*	U64 sid = CITYHASH64(type);
		auto it = m_creators.find(sid);
		if (it == m_creators.end())
			return;

		(*it)->second->setMemberData(pDest, pSrc, memberName);*/
	}

	void Manager::destroy(const TypeData *pData, void *ptr)
	{
		pData->m_destructFun(ptr);
	}
}
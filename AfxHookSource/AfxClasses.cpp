#include "stdafx.h"

#include "AfxClasses.h"
#include "addresses.h"
#include <shared/AfxDetours.h>
#include <string.h>


// CAfxMaterialKey /////////////////////////////////////////////////////////////

CAfxMaterialKey::CAfxMaterialKey(SOURCESDK::IMaterial_csgo * material)
	: m_Material(material)
{

}

CAfxMaterialKey::CAfxMaterialKey(const CAfxMaterialKey & x)
	: m_Material(x.m_Material)
{

}

CAfxMaterialKey::~CAfxMaterialKey()
{

}

SOURCESDK::IMaterial_csgo * CAfxMaterialKey::GetMaterial() const
{
	return m_Material;
}

bool CAfxMaterialKey::operator < (const CAfxMaterialKey & y) const
{
	/*
	int cmp1 = strcmp(this->m_Material->GetTextureGroupName(), y.m_Material->GetTextureGroupName());
	if(cmp1 < 0)
	return true;
	else
	if(cmp1 > 0)
	return false;

	return strcmp(this->m_Material->GetName(), y.m_Material->GetName()) < 0;
	*/

	return this->m_Material < y.m_Material;
}


// CAfxTracedMaterial //////////////////////////////////////////////////////////

std::map<SOURCESDK::IMaterial_csgo *, CAfxTrackedMaterial *> CAfxTrackedMaterial::m_Trackeds;
std::shared_timed_mutex CAfxTrackedMaterial::m_TrackedsMutex;

CAfxTrackedMaterial * CAfxTrackedMaterial::TrackMaterial(SOURCESDK::IMaterial_csgo * material)
{
	CAfxTrackedMaterial * result = 0;

	{
		std::shared_lock<std::shared_timed_mutex> shared_lock(m_TrackedsMutex);

		std::map<SOURCESDK::IMaterial_csgo *, CAfxTrackedMaterial *>::iterator it = m_Trackeds.find(material);

		if (it != m_Trackeds.end())
		{
			result = it->second;
		}
	}

	if (0 == result)
	{
		std::unique_lock<std::shared_timed_mutex> lock(m_TrackedsMutex);

		std::map<SOURCESDK::IMaterial_csgo *, CAfxTrackedMaterial *>::iterator it = m_Trackeds.find(material);

		if (it != m_Trackeds.end())
		{
			result = it->second;
		}
		else
		{
			result = new CAfxTrackedMaterial(material);

			m_Trackeds[material] = result;

			HooKVtable(material);
		}
	}

	return result;
}

std::map<int *, CAfxTrackedMaterial::CMaterialDetours> CAfxTrackedMaterial::m_VtableMap;
std::shared_timed_mutex CAfxTrackedMaterial::m_VtableMapMutex;

void CAfxTrackedMaterial::OnMaterialInterlockedDecrement(SOURCESDK::IMaterial_csgo * material)
{
	int refCount = material->Probably_GetReferenceCount();
	bool will_become_zero = 1 == refCount;

	if (will_become_zero)
	{
		std::unique_lock<std::shared_timed_mutex> lock(m_TrackedsMutex);

		std::map<SOURCESDK::IMaterial_csgo *, CAfxTrackedMaterial *>::iterator it = m_Trackeds.find(material);

		if (it != m_Trackeds.end())
		{
			it->second->Delete();
			it->second->Release();

			m_Trackeds.erase(it);
		}
	}
}

void __fastcall CAfxTrackedMaterial::Material_InterlockedDecrement(void* This, void* Edx)
{
	int * vtable = *(int**)This;

	m_VtableMapMutex.lock_shared();

	std::map<int *, CMaterialDetours>::iterator it = m_VtableMap.find(vtable);

	if (it != m_VtableMap.end())
	{
		OnMaterialInterlockedDecrement((SOURCESDK::IMaterial_csgo *) This);

		it->second.InterlockedDecrement(This, Edx);
	}
	else
		Assert(0); // should not happen.

	m_VtableMapMutex.unlock_shared();
}


void CAfxTrackedMaterial::HooKVtable(SOURCESDK::IMaterial_csgo * orgMaterial)
{
	int * vtable = *(int**)orgMaterial;

	m_VtableMapMutex.lock_shared();

	std::map<int *, CMaterialDetours>::iterator it = m_VtableMap.find(vtable);

	if (it != m_VtableMap.end())
	{
		m_VtableMapMutex.unlock_shared();
		return;
	}

	m_VtableMapMutex.unlock_shared();
	m_VtableMapMutex.lock();

	it = m_VtableMap.find(vtable);

	if (it != m_VtableMap.end())
	{
		m_VtableMapMutex.unlock();
		return;
	}

	CMaterialDetours & m_Detours = m_VtableMap[vtable];

	AfxDetourPtr((PVOID *)&(vtable[AFXADDR_GET(csgo_materialsystem_Material_InterlockedDecrement_vtable_index)]), Material_InterlockedDecrement, (PVOID *)&m_Detours.InterlockedDecrement);

	m_VtableMapMutex.unlock();
}


CAfxTrackedMaterial::CAfxTrackedMaterial(SOURCESDK::IMaterial_csgo * material)
	: CAfxMaterialKey(material)
{
}

CAfxTrackedMaterial::~CAfxTrackedMaterial()
{
#ifdef _DEBUG
	Tier0_Msg("CAfxTrackedMaterial::~CAfxTrackedMaterial 0x%08x -> %s (PRE-FREE)\n", this, m_Material->GetName());
#endif
}

void CAfxTrackedMaterial::AddNotifyee(IAfxMaterialFree * notifyee)
{
	std::unique_lock<std::mutex> lock(m_ThisNotifyeesMutex);

	if (m_Deleting)
	{
		// This path should not happen, but you never know.
		notifyee->AfxMaterialFree(this);
		return;
	}

	m_ThisNotifyees.insert(notifyee);
}

void CAfxTrackedMaterial::RemoveNotifyee(IAfxMaterialFree * notifyee)
{
	std::unique_lock<std::mutex> lock(m_ThisNotifyeesMutex);

	m_ThisNotifyees.erase(notifyee);
}

void CAfxTrackedMaterial::Delete()
{
	std::unique_lock<std::mutex> lock(m_ThisNotifyeesMutex);

	m_Deleting = true;

	while (!m_ThisNotifyees.empty())
	{
		std::set<IAfxMaterialFree*>::iterator it = m_ThisNotifyees.begin();
		IAfxMaterialFree* notifyee = *it;
		notifyee->AfxMaterialFree(this);
		m_ThisNotifyees.erase(notifyee);
	}
}

void CAfxTrackedMaterial::AddRef()
{
	++m_RefCount;
}

void CAfxTrackedMaterial::Release()
{
	--m_RefCount;

	if (0 == m_RefCount)
	{
		delete this;
	}
}


// CAfxOwnedMaterial ///////////////////////////////////////////////////////////

std::map<SOURCESDK::IMaterial_csgo *, std::atomic_int> CAfxOwnedMaterial::m_OwnedMaterials;
std::shared_timed_mutex CAfxOwnedMaterial::m_OwnedMaterialsMutex;

CAfxOwnedMaterial::CAfxOwnedMaterial(SOURCESDK::IMaterial_csgo * material)
	: CAfxMaterialKey(material)
{
	AddRef(m_Material);
}

CAfxOwnedMaterial::~CAfxOwnedMaterial()
{
	Release(m_Material);
}

void CAfxOwnedMaterial::AddRef(SOURCESDK::IMaterial_csgo * material)
{
	m_OwnedMaterialsMutex.lock_shared();

	std::map<SOURCESDK::IMaterial_csgo *, std::atomic_int>::iterator it = m_OwnedMaterials.find(material);

	if (it != m_OwnedMaterials.end())
	{
		++(it->second);
		m_OwnedMaterialsMutex.unlock_shared();
	}
	else
	{
		m_OwnedMaterialsMutex.unlock_shared();
		m_OwnedMaterialsMutex.lock();

		const std::pair<const std::map<SOURCESDK::IMaterial_csgo *,std::atomic_int>::iterator, bool> & emp = m_OwnedMaterials.try_emplace(material, 1);

		if (!emp.second)
		{
			++(emp.first->second);
		}
		else
		{
			material->IncrementReferenceCount(); // expensive operation!
		}

		m_OwnedMaterialsMutex.unlock();
	}
}

void CAfxOwnedMaterial::Release(SOURCESDK::IMaterial_csgo * material)
{
	m_OwnedMaterialsMutex.lock_shared();

	std::map<SOURCESDK::IMaterial_csgo *, std::atomic_int>::iterator it = m_OwnedMaterials.find(material);

	if (it != m_OwnedMaterials.end())
	{
		int lastValue = --(it->second);
		m_OwnedMaterialsMutex.unlock_shared();

		if (0 == lastValue)
		{
			m_OwnedMaterialsMutex.lock();

			std::map<SOURCESDK::IMaterial_csgo *, std::atomic_int>::iterator it = m_OwnedMaterials.find(material);

			if (it != m_OwnedMaterials.end())
			{
				if (0 == it->second)
				{
					m_OwnedMaterials.erase(it);
					material->DecrementReferenceCount(); // expensive operation!
				}
			}

			m_OwnedMaterialsMutex.unlock();
		}
	}
	else
	{
		m_OwnedMaterialsMutex.unlock_shared();
		throw "void CAfxOwnedMaterial::Release(SOURCESDK::IMaterial_csgo * material): Unexpected call.";
	}
}

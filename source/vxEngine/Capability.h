#pragma once

namespace Graphics
{
	class CapabilitySetting;

	class Capability
	{
	protected:
		CapabilitySetting* m_pSetting{nullptr};

	public:
		Capability() = default;
		explicit Capability(CapabilitySetting* setting) :m_pSetting(setting){}

		virtual ~Capability(){}

		virtual void set() = 0;
	};
}
#pragma once
#include "./AnthemComponentsBaseImports.h"

namespace Anthem::Component {
	class AnthemComponent;
	class AnthemEntity;

	class AnthemComponent {
	public:
		AnthemEntity* entity;

		virtual void onCreate() {}
		virtual void onStart() {}
		virtual void onUpdate() {}
		virtual void onPreRender() {}
		virtual void onPostRender() {}
	};

	class AnthemEntity {
	public:
		using AtIdentityIdentifier = int;
		std::unordered_map<AtIdentityIdentifier, std::unique_ptr<AnthemComponent>> attachedComponents;
		AtIdentityIdentifier curId = 0;

		template<typename T,typename... U> T* addComponent(U... args){
			attachedComponents[curId++] = std::make_unique<T>(args...);
			(attachedComponents[curId - 1]).get()->entity = this;
			return attachedComponents[curId-1].get();
		}
		template<typename T> std::vector<T*> getComponents() {
			std::vector<T*> ret;
			for (auto& [k,v] : attachedComponents) {
				if (typeid(*(v.get())) == typeid(T)) {
					ret.push_back(v.get());
				}
			}
			return ret;
		}
	};
}
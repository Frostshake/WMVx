#pragma once
#include <type_traits>
#include <qobject.h>

namespace core {

	template<typename T>
	class Singleton {
	public:
		static void boot(QObject* parent) {
			static_assert(std::is_base_of<QObject, T>::value, "T must be derived fro QObject");
			_instance = new T(parent);
		}

		static T* instance() {
			return _instance;
		}

	protected:
		Singleton() = delete;

		static T* _instance;
	};
}
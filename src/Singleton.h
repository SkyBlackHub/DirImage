#ifndef Singleton_H
#define Singleton_H

#include <cassert>

namespace DIP {

	template <class C>
	class SingletonDefault;

	template <class C>
	class Singleton
	{
	public:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(Singleton&&) = delete;

		~Singleton()
		{
			s_instance = nullptr;
		}

		static bool isInitialized()
		{
			return s_instance != nullptr;
		}

		template <typename... Args>
		static C& initialize(Args... args)
		{
			if (s_instance == nullptr) {
				s_instance = new C(args...);
				s_instance->bootstrap();
			}
			return *s_instance;
		}

		static void deinitialize()
		{
			if (s_instance != nullptr) {
				delete s_instance;
				s_instance = nullptr;
			}
		}

		template <typename... Args>
		static C& reinitialize(Args... args)
		{
			deinitialize();
			return initialize(args...);
		}

		static C& instance()
		{
			if (s_instance == nullptr) {
				assert(("Calling an uninitialized Singleton instance."));
			}
			return *s_instance;
		}

	protected:
		Singleton() = default;

		static C* instancePointer()
		{
			return s_instance;
		}

		static void setInstancePointer(C *instance)
		{
			if (s_instance != instance) {
				delete s_instance;
				s_instance = instance;
			}
		}

		virtual void bootstrap() { }

	private:
		static C* s_instance;

		friend class SingletonDefault<C>;
	};

	template <class C>
	class SingletonDefault : public Singleton<C>
	{
	public:
		static C& instance()
		{
			return initialize();
		}
	};
}

template <class C>
C* DIP::Singleton<C>::s_instance = nullptr;

#endif // Singleton_H
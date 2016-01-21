#pragma once

namespace BitEngine
{
	// forward declare
	template<typename T> class SmartManager;
	template<typename T> class SmartPointer;

	template<typename T>
	class SmartPointerSource
	{
	public:
		/**
		 * Use selfCount = 1 for caching an extra instance reference
		 */
		SmartPointerSource(T* p, SmartManager<T>*const manager, int selfCount=0)
			: obj(p), myManager(manager), myOwnCount(selfCount)
		{
			count = new int(selfCount);
		}

		~SmartPointerSource()
		{
			if (myOwnCount > 0)
			{
				(*count) -= myOwnCount;
				if ((*count) == 0)
				{
					myManager->DeleteReference(obj);
				}
				myOwnCount = -1; // Invalidate self, avoid deadlocks
			}
		}

		T* operator->(void)
		{
			return obj;
		}

		const T* operator->(void) const
		{
			return obj;
		}

		int getCount() const {
			return *count;
		}

		void DeleteReference()
		{
			myOwnCount = -1; // mark self as invalid, avoid deadlocks
			delete count;
			count = nullptr;
			myManager->DeleteReference(obj);
		}

	private:
		friend class SmartPointer<T>;

		T* obj;
		int *count;
		SmartManager<T>* myManager;
		int myOwnCount;
	};

	template<typename T>
	class SmartPointer
	{
	public:
		SmartPointer()
			: obj(nullptr), mySource(nullptr)
		{}
		
		SmartPointer(const SmartPointer& copy)
		{
			// Reference counting
			if (copy.valid())
			{
				++(*(copy.mySource->count));
				mySource = copy.mySource;
				obj = copy.obj;
			} else {
				mySource = nullptr;
				obj = nullptr;
			}
		}

		SmartPointer(SmartPointerSource<T>& copy)
		{
			if (copy.obj != nullptr)
			{
				++(*(copy.count));
				mySource = &copy;
				obj = copy.obj;
			}
			else
			{
				mySource = nullptr;
				obj = nullptr;
			}
		}

		~SmartPointer()
		{
			KillMyReference();
		}


		SmartPointer<T>& operator=(const SmartPointer<T>& other)
		{
			if (other.obj == nullptr)
			{
				// Set myself to an invalid instance
				KillMyReference();
			}
			else
			{
				++(*(other.mySource->count));

				mySource = other.mySource;
				obj = other.obj;
			}

			return *this;
		}

		const SmartPointer<T>& operator=(SmartPointerSource<T>& other)
		{
			if (other.obj == nullptr)
			{
				// Set myself to an invalid instance
				KillMyReference();
			}
			else
			{
				++(*(other.count));

				obj = other.obj;
				mySource = &other;
			}

			return *this;
		}

		T* operator->(void)
		{
			return obj;
		}

		const T* operator->(void) const
		{
			return obj;
		}

		bool operator <(const SmartPointer<T>& other) const{
			return obj < other.obj;
		}

		bool operator ==(const SmartPointer<T>& other) const{
			return obj == other.obj;
		}

		bool valid() const{
			return obj != nullptr;
		}

		int getCount() const{
			return mySource->getCount();
		}

	private:

		void KillMyReference()
		{
			if (obj != nullptr)
			{
				if (--(*mySource->count) == 0){
					mySource->DeleteReference();
				}
			}

			mySource = nullptr;
			obj = nullptr;
		}

		T* obj;
		SmartPointerSource<T>* mySource;
	};

	template<typename T>
	class SmartManager
	{
	public:
		virtual void DeleteReference(T* reference) = 0;
	};

}
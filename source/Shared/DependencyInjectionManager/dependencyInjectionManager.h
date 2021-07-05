#ifndef _DEPENDENCY_INJECTION_MANAGER_H_
#define _DEPENDENCY_INJECTION_MANAGER_H_
#include <vector>
#include <map>
#include <functional>
#include <string>

#ifdef __TESTING__
    #include <tester.h>
#endif

//note, may use an Interface instead void*



using namespace std;

struct OnDemandInstance{
	function<void*()> createInstance;
	void* instance;
};

class DependencyInjectionManager{
private:
#ifdef __TESTING__
    public:
#endif
	map<string, OnDemandInstance> singletons;
	map<string, function<void*()>> multiInstance;
public:
	
	~DependencyInjectionManager()
	{
		for (auto &c: singletons)
		{
			if (c.second.instance != NULL)
				delete c.second.instance;
		}
	}

	void addSingleton(string name, function<void*()> createInstance, bool instantiateImediately)
	{
		if (multiInstance.count(name) > 0)
			multiInstance.erase(name);
		
		OnDemandInstance p;
		p.createInstance = createInstance;
		p.instance = NULL;
		
		if (instantiateImediately)
			p.instance = createInstance();
		
		singletons[name] = p;
	}
	
	///Pre instantiated singletons. Very util to create hosted services.
	template <class T>
	void addSingleton(string name, T* instance)
	{
		this->addSingleton(name, [instance](){
			return (void*)instance;
		}, true);
	}
	
	///Create the singleton only when needed
	template <class T>
	void addSingleton(string name, function<T*()> createInstance)
	{
		this->addSingleton(name, [createInstance](){
			return (void*)createInstance();
		}, false);
	}	

	template <class T>
	void addMultiInstance(string name, function<T*()> createInstance)
	{
		if (singletons.count(name) > 0)
			singletons.erase(name);
		
		multiInstance[name] = [createInstance](){
			return (void*)createInstance();
		};
	}	

	
	template <class T>
	T* get(string name)
	{
		if (singletons.count(name) > 0)
		{
			if (singletons[name].instance == NULL)
				singletons[name].instance = singletons[name].createInstance();
			
			return (T*)singletons[name].
			instance;
			
		}
		else if (multiInstance.count(name) > 0)
		{
			return (T*)multiInstance[name]();
			
		}
		else
			return NULL;
	}
	
	template <class T>
	void putBack(string name, T* instance)
	{
		if (multiInstance.count(name) > 0)
			delete instance;
	}
};




/*auto di = new DependencyInjectionManager();
di->addSingleton<IController>("controller", new Controller());

di->addSingleton<IAPI>("api", new API(di->get<IController>("controller")));

class IAPI{};
class API2: public IAPI{
public:
	IController controller;
	API2(DependencyInjectionManager* di)
	{
		this->controller = di->get<IController>("controller"); 
		
	}
}

di->addSingleton<IAPI>("api2", new API2(di));*/

#endif
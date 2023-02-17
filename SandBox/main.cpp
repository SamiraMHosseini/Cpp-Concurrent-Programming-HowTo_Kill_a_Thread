#include <conio.h>
struct SharedResource
{
	SharedResource() :
		mtx(), flag(false)
	{

	}
	//These are resources that are shared between 
	std::mutex mtx;
	std::condition_variable cv;
	bool flag;
};
class A : public BannerBase
{

public:
	A() = delete;
	A(const A&) = default;
	A& operator=(const A&) = default;
	~A() = default;
	A(const char* const name) :
		counter(0), name(name), BannerBase(name)
	{

	}
	void operator ()(SharedResource& sharedresource) //Functor 
	{
		START_BANNER;
		Debug::SetCurrentName(name);
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedresource.mtx);
			//Wait for 500ms unless there is an interrupt or a signal
			if (sharedresource.cv.wait_for(lock, 500ms, [&]() -> bool { return sharedresource.flag; }))
			{
				break;
			}

			Debug::out("%d \n", counter);
			++counter;


		}

	}

private:
	int counter;
	const char* const name;
};



int main()
{
	START_BANNER_MAIN("Main");

	A a("A");

	//Resources shared between threads 
	SharedResource sharedresource;

	//Spawn a thread 
	std::thread thA(a, std::ref(sharedresource));
	thA.detach();

	//Key Press
	_getch();

	//Signal to the thread 
	{
		
		Debug::out("Key is pressed\n");
		std::unique_lock<std::mutex> lock(sharedresource.mtx);
		sharedresource.flag = true;
		sharedresource.cv.notify_one();
	}
	std::this_thread::sleep_for(1s); //This is going to get fixed in future versions. 
}
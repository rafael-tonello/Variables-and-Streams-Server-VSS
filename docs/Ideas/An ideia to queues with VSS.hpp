using OnMessage = function<void (vector<JSON> messages)>
class VSS_Queue
{
private:
	shared_ptr<VSS> vss;
	string queueNameON;

	vector<OnMessage> observers;

	void readAndNotify()
	{
		//observate every deletes the messages
		if (observers.size() > 0)
		{
			auto result = read(INT_MAX, true);
			for (auto &c: observers)
				c(result);
		}
	}
public:
	VSS_Queue(string queueNameON, shared_ptr<VSS> vss)
	{
		this->vss = vss;
		this->queueNameON = queueNameON;


		vss->observate(queueNameON + ".count", [&](DynamicVar newValue)
		{
			readAndNotify();
		});
	}

	//observate every deletes the messages
	void observate(OnMessage observer)
	{
		observers.push_back(observer);
	}
	
	vector<JSON> read(int max, bool deleteMessages = true)
	{
		vector<JSON> result;
		//lock the queue
		vss->lock(queueNameON + ".lock").get();
		
		int maxItems = vss->getVar(queueNameON + ".count").get().asInt();
		max = maxItems < max ? maxItems: max;

		int totalRead = vss->getVar(queueNameON + ".read").get().asInt();

		if (deleteMessages)
			vss->setVar(queueNameON + ".read", DynamicVar(totalRead+max);

		//unlock the queue
		vss->unlock(queueNameON + ".lock");
		
		//read the messages and delete them
		for (int c = toralRead; c < maxItems; c++)
		{
			result.push_back(JSON(vss->getVar(queueNameON + "." + to_string(c), "{}").get().asString()));
			
			if (deleteMessages)
				vss->delVar(queueNameON + "." + to_string(c));

			if (c - totalRead > max)
				break;
		}

		
		return read;
	}
	
	
	void write(JSON data)
	{
		//lock the queue
		vss->lock(queueNameON + ".lock").get();
		
		//update messages count
		string vName = queueNameON + ".count";
		count = vss->getVar(vName, 0);
		vss->setVar(vName, count+1);
		
		//save the value_comp
		vss->setVar(queueNameON + "." + to_string(count), data.toString());
		
		//unlock the queue
		vss->unlock(queueNameON + ".lock");
		
	}
}
#include "TCPServer.h"

#pragma region TCPServer class
	#pragma region private functions
		void TCPServerLib::TCPServer::notifyListeners_dataReceived(ClientInfo *client, char* data, size_t size)
		{

		}

		void TCPServerLib::TCPServer::notifyListeners_connEvent(ClientInfo *client, CONN_EVENT action)
		{
			//IMPORTANT: if disconnected, the 'client' must be destroyed here (or in the function that calls this function);
		}

		void TCPServerLib::TCPServer::initialize(vector<int> ports, ThreadPool* tasker)
		{
			if (tasker == NULL)
				tasker = new ThreadPool();
			this->__tasks = tasker;

			for (auto &p: ports)
			{
				thread *th = new thread([this,p](){
					this->listenClients(p);
				});

				

				th->detach();
				
				this->listenThreads.push_back(th);
			}

			thread *th2 = new thread([this](){
				this->talkWithClients();
			});

			th2->detach();
		}

		void TCPServerLib::TCPServer::listenClients(int port)
		{
			//create an socket to await for connections

			int listener;

			struct sockaddr_in *serv_addr = new sockaddr_in();
			struct sockaddr_in *cli_addr = new sockaddr_in();
			int status;
			socklen_t clientSize;

			listener = socket(AF_INET, SOCK_STREAM, 0);

			if (listener >= 0)
			{
				int reuse = 1;
				if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
				this->debug("setsockopt(SO_REUSEADDR) failed");
				//fill(std::begin(serv_addr), std::end(serv_addr), T{});
				//bzero((char *) &serv_addr, sizeof(serv_addr));
				//setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &iOptVal, &iOptLen);

				serv_addr->sin_family = AF_INET;
				serv_addr->sin_addr.s_addr = INADDR_ANY;
				serv_addr->sin_port = htons(port);

				usleep(1000);
				status = bind(listener, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
				usleep(1000);
				if (status >= 0)
				{
					//SetSocketBlockingEnabled(listener, false);
					status = listen(listener, 5);
					if (status >= 0)
					{

						clientSize = sizeof(cli_addr);

						this->debug("The server is listening and waiting for news connections");
						while (true)
						{
							int theSocket = accept(listener, (struct sockaddr *) cli_addr, &clientSize);

							//int client = accept(listener, 0, 0);

							if (theSocket >= 0)
							{
								//creat ea new client
								ClientInfo client;
								client.socketHandle = theSocket;
								client.server = this;

								this->connectedClients[theSocket](client);
							}
							else{
								usleep(5000);
							}

						}
					}
					else
						this->debug("Failure to open socket");
				}
				else
					this->debug("Failure to start socket system");
			}
			else
				this->debug("General error opening socket");

				//n = read(newsockfd,buffer,255);
				//n = write(newsockfd,"I got your message",18);
		}

		void TCPServerLib::TCPServer::talkWithClients()
		{
			//scrolls the list of clients and checks if exists data to be read

				//when some data is read, send it to the server observer and respective client observers

		}

	#pragma endregion

	#pragma region public functions


	TCPServerLib::TCPServer::TCPServer(int port, ThreadPool *tasker)
	{
		vector<int> ports = {port};
		this->initialize(ports, tasker);
	}

	TCPServerLib::TCPServer::TCPServer(vector<int> ports, ThreadPool *tasker)
	{
		this->initialize(ports, tasker);
	}

	TCPServerLib::TCPServer::~TCPServer()
	{

	}

	void TCPServerLib::SocketHelper::addReceiveListener(function<void(ClientInfo *client, char* data,  size_t size)> onReceive)
	{
		this->receiveListeners.push_back(onReceive)
	}

	void TCPServerLib::SocketHelper::addReceiveListener_s(function<void(ClientInfo *client, string data)> onReceive)
	{
		this->receiveListeners_s.push_back(onReceive);
	}

	void TCPServerLib::SocketHelper::addConEventListeners(function<void(ClientInfo *client, CONN_EVENT event)> onConEvent)
	{
		this->connEventsListeners.push_back(onConEvent);
	}

	void TCPServerLib::TCPServer::send(ClientInfo *client, char* data, size_t size)
	{ 
		client->writeMutex.lock();



		client->writeMutex.unlock();
	}

	void TCPServerLib::TCPServer::send(ClientInfo *client, string data)
	{
		client->writeMutex.lock();

		client->writeMutex.unlock();
	}

	void TCPServerLib::TCPServer::sendBroadcast(char* data, size_t size, vector<ClientInfo*> *clientList)
	{
		if (clientList == NULL)
		{
			vector<ClientInfo*> temp;
			for (auto &c: this->connectedClients)   
				temp.push_back(&(c.second));
			clientList = &temp;
		}
		for (int c = 0; c < clientList->size(), c++)
		{
			this->__tasks->enqueue([data, size](ClientInfo* p){
				p->send(data, size);
			
			}, (*clientList)[c]);
		}

	}

	void TCPServerLib::TCPServer::sendBroadcast(string data, vector<ClientInfo*> *clientList)
	{
		this->sendBroadcast((char*)(data.c_str()), data.size(), clientList);
	}


	#pragma endregion
#pragma endregion
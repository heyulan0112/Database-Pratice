#include "connectpool.h"
#include <stdexcept>
#include <exception>
#include <stdio.h>

using namespace std;
using namespace sql;

ConnectPool *ConnectPool::connPool = NULL;

ConnectPool::ConnectPool(string urll, string username, string pw, int maxSize)//连接池的构造函数
{
	this->currentSize = 0;
	this->url = urll;//默认连接的url
	this->user = username;//默认连接的user
	this->password = pw;//默认连接的密码
	this->maxSize = maxSize;
	try 
	{
		this->driver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException&e) 
	{
		perror("驱动连接出错了;\n");
	}
	catch (std::runtime_error&e) 
	{
		perror("运行出错了\n");
	}
	this->InitConnection(maxSize / 2);
}
Connection* ConnectPool::CreateConnection() 
{
	Connection* conn;
	try 
	{
		conn = driver->connect(this->url,this->user,this->password); //建立连接jdbc:mysql://localhost:3306/mysql,创建的都是默认的连接
		return conn;
	}
	catch (sql::SQLException&e) 
	{
		perror("创建连接出错");
		return NULL;
	}
	catch (std::runtime_error&e) 
	{
		perror("运行时出错");
		return NULL;
	}
}
Connection* ConnectPool::CreateConnection(string urll, string username, string pw) //connection是mysql自带的
{
	Connection* conn;
	try 
	{
		this->url = urll;
		this->user = username;
		this->password = pw;
		conn = driver->connect(this->url,this->user,this->password); 
		return conn;
	}
	catch (sql::SQLException&e) 
	{
		perror("创建连接出错");
		return NULL;
	}
	catch (std::runtime_error&e) 
	{
		perror("运行时出错");
		return NULL;
	}
}
void ConnectPool::InitConnection(int iInitialSize) //初始化两个用户,参数可以省略
{
	Connection* conn1;
	Connection* conn2;
	try
	{
		conn1 = driver->connect("tcp://127.0.0.1:3306", "root", "system");//conn代表一个连接,在新连接池里创建连接}
	}
	catch (sql::SQLException&e) 
	{
		conn1 = NULL;
	}
	catch (std::runtime_error&e) 
	{
		conn1 = NULL;
	}	
	if (conn1!=NULL) 
	{
		connList.push_back(conn1);//将连接池里未分配的连接都放进connList中
		this->currentSize=this->currentSize+1;
	}
	else 
	{
		perror("创建连接1出错了");
	}
	try
	{
		conn2 = driver->connect("tcp://127.0.0.1:3306", "scoot", "tiger");
	}
	catch (sql::SQLException&e) 
	{
		conn2 = NULL;
	}
	catch (std::runtime_error&e) 
	{
		conn2 = NULL;
	}
	if (conn2!=NULL)
	{
		connList.push_back(conn2);//将连接池里未分配的连接都放进connList中
		this->currentSize = this->currentSize + 1;
	}
	else 
	{
		perror("创建连接2出错了");
	}
}
//销毁一个连接
void ConnectPool::DestoryConnection(Connection* conn) 
{
	if (conn) 
	{
		try 
		{
			conn->close();
		}
		catch (sql::SQLException&e) 
		{
			perror(e.what());
		}
		catch (std::exception&e) 
		{
			perror(e.what());
		}
		delete conn;
	}
}

//从连接池中获取连接实例
Connection* ConnectPool::GetConnection() 
{ 
	Connection* con;
	if (connList.size() > 0) //先从connList获取连接
	{   //连接池容器中还有连接
		con = connList.front(); //得到第一个连接
		connList2.push_back(con);//将得到的连接放入connList2
		connList.pop_front();   //移除第一个连接
		if (con->isClosed())
		{   
			//如果连接已经被关闭，删除后重新建立一个
			delete con;
			con = this->CreateConnection();
		}
		//如果连接为空，则获取连接出错
		if (con == NULL) 
		{
			currentSize--;
		}
	}
	else//connList中没有连接时，在条件允许下自己创建新连接
	{
		if (currentSize < maxSize) 
		{ //还可以创建新的连接
			con = this->CreateConnection();
			if (con!=NULL)
			{
				currentSize++;
			}
		}
		else 
		{   //建立的连接数已经达到maxSize
			cout << "连接已达上限！" << endl;
			return NULL;
		}
	}
	return con;
}

//回收数据库连接
void ConnectPool::ReleaseConnection(sql::Connection * conn) 
{
	if (conn) 
	{
		connList.push_back(conn);
		connList2.pop_front();
	}
}

//连接池的析构函数
ConnectPool::~ConnectPool() 
{
	list<Connection*>::iterator icon;
	for (icon = connList.begin(); icon != connList.end(); ++icon) 
	{
		this->DestoryConnection(*icon); //销毁连接池中未分配的连接
	}
	for (icon = connList2.begin(); icon != connList2.end(); ++icon) 
	{
		this->DestoryConnection(*icon); //销毁已分配的连接
	}
	currentSize = 0;//将连接池大小归零
	connList.clear(); //清空未分配list
	connList2.clear();//清空已分配list
}

ConnectPool* ConnectPool::GetInstance()//获取连接池实例
{
	if (connPool == NULL) 
	{
		connPool = new ConnectPool("tcp://127.0.0.1:3306", "root", "system", 50);//为空时创建根用户
	}
	return connPool;//返回连接池实例
}


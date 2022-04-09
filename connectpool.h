#ifndef _CONNECTION_POOL_H
#define _CONNECTION_POOL_H

#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <pthread.h>
#include <list>

using namespace std;
using namespace sql;
#endif
class ConnectPool 
{
private:
	int currentSize; //当前已建立的数据库连接数量
	int maxSize; //连接池中定义的最大数据库连接数
	string user;
	string password;
	string url;
	list<Connection*> connList; //连接池的容器队列
	list<Connection*> connList2;//连接池中已经分配了的容器队列
	static ConnectPool *connPool;//连接池指针
	Driver* driver;
	ConnectPool(string url, string user, string password, int maxSize); //构造方法
	Connection* CreateConnection(); //创建一个连接
	Connection* CreateConnection(string urll, string username, string pw);
	void InitConnection(int iInitialSize); //初始化数据库连接池
	void DestoryConnection(Connection *conn); //销毁数据库连接对象
public:
	~ConnectPool();
	Connection* GetConnection(); //获得数据库连接
	void ReleaseConnection(Connection *conn); //将数据库连接放回到连接池的容器中
	static ConnectPool* GetInstance(); //获取数据库连接池对象
};

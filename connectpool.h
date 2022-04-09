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
	int currentSize; //��ǰ�ѽ��������ݿ���������
	int maxSize; //���ӳ��ж����������ݿ�������
	string user;
	string password;
	string url;
	list<Connection*> connList; //���ӳص���������
	list<Connection*> connList2;//���ӳ����Ѿ������˵���������
	static ConnectPool *connPool;//���ӳ�ָ��
	Driver* driver;
	ConnectPool(string url, string user, string password, int maxSize); //���췽��
	Connection* CreateConnection(); //����һ������
	Connection* CreateConnection(string urll, string username, string pw);
	void InitConnection(int iInitialSize); //��ʼ�����ݿ����ӳ�
	void DestoryConnection(Connection *conn); //�������ݿ����Ӷ���
public:
	~ConnectPool();
	Connection* GetConnection(); //������ݿ�����
	void ReleaseConnection(Connection *conn); //�����ݿ����ӷŻص����ӳص�������
	static ConnectPool* GetInstance(); //��ȡ���ݿ����ӳض���
};

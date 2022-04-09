#include "connectpool.h"
#include <stdexcept>
#include <exception>
#include <stdio.h>

using namespace std;
using namespace sql;

ConnectPool *ConnectPool::connPool = NULL;

ConnectPool::ConnectPool(string urll, string username, string pw, int maxSize)//���ӳصĹ��캯��
{
	this->currentSize = 0;
	this->url = urll;//Ĭ�����ӵ�url
	this->user = username;//Ĭ�����ӵ�user
	this->password = pw;//Ĭ�����ӵ�����
	this->maxSize = maxSize;
	try 
	{
		this->driver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException&e) 
	{
		perror("�������ӳ�����;\n");
	}
	catch (std::runtime_error&e) 
	{
		perror("���г�����\n");
	}
	this->InitConnection(maxSize / 2);
}
Connection* ConnectPool::CreateConnection() 
{
	Connection* conn;
	try 
	{
		conn = driver->connect(this->url,this->user,this->password); //��������jdbc:mysql://localhost:3306/mysql,�����Ķ���Ĭ�ϵ�����
		return conn;
	}
	catch (sql::SQLException&e) 
	{
		perror("�������ӳ���");
		return NULL;
	}
	catch (std::runtime_error&e) 
	{
		perror("����ʱ����");
		return NULL;
	}
}
Connection* ConnectPool::CreateConnection(string urll, string username, string pw) //connection��mysql�Դ���
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
		perror("�������ӳ���");
		return NULL;
	}
	catch (std::runtime_error&e) 
	{
		perror("����ʱ����");
		return NULL;
	}
}
void ConnectPool::InitConnection(int iInitialSize) //��ʼ�������û�,��������ʡ��
{
	Connection* conn1;
	Connection* conn2;
	try
	{
		conn1 = driver->connect("tcp://127.0.0.1:3306", "root", "system");//conn����һ������,�������ӳ��ﴴ������}
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
		connList.push_back(conn1);//�����ӳ���δ��������Ӷ��Ž�connList��
		this->currentSize=this->currentSize+1;
	}
	else 
	{
		perror("��������1������");
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
		connList.push_back(conn2);//�����ӳ���δ��������Ӷ��Ž�connList��
		this->currentSize = this->currentSize + 1;
	}
	else 
	{
		perror("��������2������");
	}
}
//����һ������
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

//�����ӳ��л�ȡ����ʵ��
Connection* ConnectPool::GetConnection() 
{ 
	Connection* con;
	if (connList.size() > 0) //�ȴ�connList��ȡ����
	{   //���ӳ������л�������
		con = connList.front(); //�õ���һ������
		connList2.push_back(con);//���õ������ӷ���connList2
		connList.pop_front();   //�Ƴ���һ������
		if (con->isClosed())
		{   
			//��������Ѿ����رգ�ɾ�������½���һ��
			delete con;
			con = this->CreateConnection();
		}
		//�������Ϊ�գ����ȡ���ӳ���
		if (con == NULL) 
		{
			currentSize--;
		}
	}
	else//connList��û������ʱ���������������Լ�����������
	{
		if (currentSize < maxSize) 
		{ //�����Դ����µ�����
			con = this->CreateConnection();
			if (con!=NULL)
			{
				currentSize++;
			}
		}
		else 
		{   //�������������Ѿ��ﵽmaxSize
			cout << "�����Ѵ����ޣ�" << endl;
			return NULL;
		}
	}
	return con;
}

//�������ݿ�����
void ConnectPool::ReleaseConnection(sql::Connection * conn) 
{
	if (conn) 
	{
		connList.push_back(conn);
		connList2.pop_front();
	}
}

//���ӳص���������
ConnectPool::~ConnectPool() 
{
	list<Connection*>::iterator icon;
	for (icon = connList.begin(); icon != connList.end(); ++icon) 
	{
		this->DestoryConnection(*icon); //�������ӳ���δ���������
	}
	for (icon = connList2.begin(); icon != connList2.end(); ++icon) 
	{
		this->DestoryConnection(*icon); //�����ѷ��������
	}
	currentSize = 0;//�����ӳش�С����
	connList.clear(); //���δ����list
	connList2.clear();//����ѷ���list
}

ConnectPool* ConnectPool::GetInstance()//��ȡ���ӳ�ʵ��
{
	if (connPool == NULL) 
	{
		connPool = new ConnectPool("tcp://127.0.0.1:3306", "root", "system", 50);//Ϊ��ʱ�������û�
	}
	return connPool;//�������ӳ�ʵ��
}


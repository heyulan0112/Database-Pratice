#include "connectpool.h"
#include "stdio.h"
#include "mysql.h"
#include <windows.h>
#include <winsock.h>
#include <WinUser.h>
#include <iostream>
#include <pthread.h>
#include <mutex>
#include<cstring>
#pragma comment(lib, "libmysql.lib")
#define NUM_THREADS 2

using namespace std;
mutex mu;
ConnectPool *connpool = ConnectPool::GetInstance();
////////////多用户访问连接user1//////////////
void user01()
{
	mu.lock(); //加锁,共享数据在同一时刻只被一个线程使用
	Connection *con;
	Statement *state;//Statement是在jdbc中向数据库传递SQL语句的类型
	ResultSet *result;//结果集，用于存放SQL语句的执行结果

	con = connpool->GetConnection();//从连接池中获取一个连接
	state = con->createStatement();
	state->execute("use mysql");
	result = state->executeQuery("select id from mysql.student");//在mysql 数据库中执行select id from mysql.student语句
	if (result->next() != false) 
	{
		while (result->next())
		{
			int id = result->getInt("id");
			cout << "user1执行select id from mysql.student结果为:" << id << endl;
		}
	}
	delete state;
	connpool->ReleaseConnection(con);//释放连接，放回连接池
	mu.unlock(); //解除锁
}
////////////多用户访问连接user2//////////////
void user02()
{
	mu.lock();
	Connection *con;
	Statement *state;
	ResultSet *result;

	con = connpool->GetConnection();
	state = con->createStatement();
	state->execute("use mysql");
	result = state->executeQuery("select * from student where id < 1002");
	if (result->next() != false)
	{
		while (result->next())
		{
			int id = result->getInt("id");
			cout << "user2执行select * from student where id < 1002结果为:" << id << endl;
		}
	}
	delete state;
	connpool->ReleaseConnection(con);
	mu.unlock();

}
////////////格式输出数据表//////////////
bool output(MYSQL *connetion) {
	MYSQL_RES *res;
	MYSQL_ROW column;
	int count = 0;
	res = mysql_store_result(connetion);
	if (!res) {
		printf("Couldn't get result from %s\n", mysql_error(connetion));
		return false;
	}
	int j = mysql_num_fields(res);
	char *str_field[32];
	for (int i = 0; i < j; i++) {
		str_field[i] = mysql_fetch_field(res)->name;
	}
	for (int i = 0; i < j; i++)
		printf("%-15s\t", str_field[i]);
	printf("\n");
	while (column = mysql_fetch_row(res)) {
		for (int i = 0; i < j; ++i) {
			printf("%-15s\t", column[i]);
		}
		printf("\n");
		count++;
	}
	printf("有%d条记录\n", count);
	return true;
}
////////////插入//////////////
void insert(char *s1, char *s2, int n)
{
	int len1 = 0, len2 = 0, j = 0, len3, k = 0;
	char s4[100];
	len1 = strlen(s1);
	len2 = strlen(s2);
	for (int i = 0; i < n; i++)
	{
		j++;
	}
	for (int i = 0; i < len1; i++)
	{
		s4[k++] = s1[i];
	}
	for (int i = 0; i < len2; i++)
		s1[j++] = s2[i];

	for (int i = n; i < len1; i++)
		s1[j++] = s4[i];

	s1[j] = '\0';
}
////////////表连接（连接显示数据表）//////////////
void join(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	char tmp2[400];
	int x = 0, y = 0, z = 0, rt;
	int count = 0;
	int t = 0;
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}

	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		for (int t = 0; t < mysql_num_fields(res); t++) {
			x++;
			cout << x << ".";
			printf("%s  ", row[t]);
		}
		cout << endl;
	}//输出所有表以及各个表的序号
	cout << "输入要连接的左方的表的序号：";
	cin >> x;
	cout << "\n输入要连接的右方的表的序号：";
	cin >> y;
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x)
		{
			sprintf_s(tmp1, row[0]);
			if (z == 0) z++;
			else break;
		}
		if (t == y)
		{
			sprintf_s(tmp2, row[0]);
			if (z == 0) z++;
			else break;
		}
	}//找到选定的两张表的表名，存入tmp1和tmp2中
	cout << endl;
	cout << "1:内连接\n";
	cout << "2:左连接\n";
	cout << "3:右连接\n";
	cout << "输入连接方式的序号：";
	cin >> z;
	switch (z)
	{
	case 1:
		sprintf_s(tmp, "select * from  join  on ");
		insert(tmp, tmp1, 14);
		insert(tmp, tmp2, strlen(tmp) - 4);
		break;
	case 2:
		sprintf_s(tmp, "select * from  left join  on ");
		insert(tmp, tmp1, 14);
		insert(tmp, tmp2, strlen(tmp) - 4);
		break;
	case 3:
		sprintf_s(tmp, "select * from  right join  on ");
		insert(tmp, tmp1, 14);
		insert(tmp, tmp2, strlen(tmp) - 4);
		break;
	}
	printf("输入连接条件：");
	cin.get();
	cin.get(tmp1, 100);
	insert(tmp, tmp1, strlen(tmp));
	cout << tmp << endl;
	system("cls");

	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}

	output(con);
	mysql_free_result(res);
	system("pause");
	system("cls");
}
////////////新建数据表//////////////
void create(MYSQL * con) {
	char tmp[400];//tmp存完整SQL语句
	char tmp1[400];//tmp1存临时输入结果
	int x, rt;
	sprintf_s(tmp, "create table ()");
	printf("输入要新建的表名：");
	cin.get();
	cin.get(tmp1, 100);
	insert(tmp, tmp1, 13);
	printf("输入字段名和字段类型：");
	cin.get();
	cin.get(tmp1, 100);
	x = strlen(tmp) - 1;
	insert(tmp, tmp1, x);
	cout << tmp << endl;//输出SQL语句
	rt = mysql_real_query(con, tmp, strlen(tmp));//执行tmp存的SQL语句，成功函数返回0
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else
	{
		printf("创建成功!!!\n");
	}
	system("pause");
	system("cls");
};
////////////显示所有数据表//////////////
void show2(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int t = 0;
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));//执行show tables语句
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {

		for (int t = 0; t < mysql_num_fields(res); t++) {
			x++;
			cout << x << ".";
			printf("%s  ", row[t]);
		}
		cout << endl;
	}
}
////////////显示数据表内容(单表显示数据表)//////////////
void show(MYSQL * con){
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int count = 0;
	int t = 0;
	show2(con);
	cout << "输入要查看的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中
	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}//找到选定行的表名存入tmp1中
	sprintf_s(tmp1, row[0]);
	sprintf_s(tmp, "select * from ");
	insert(tmp, tmp1, 14);
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	output(con);
	mysql_free_result(res);
	system("pause");
	system("cls");
}
////////////显示数据表内容//////////////
void show1(MYSQL * con){
	int i;
	cout << "1:单表显示\n";
	cout << "2:连接显示\n";
	cout << "输入要选择的显示方式的序号：";
	cin >> i;
	system("cls");
	switch (i){
	case 1:
		show(con);
		break;

	case 2:
		join(con);
		break;
	}
}
////////////插入数据//////////////
void insert1(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int t = 0;
	show2(con);
	cout << "输入要插入的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中
	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);//找到需插入表的表名存入tmp1
	sprintf_s(tmp, "insert into  values ()");
	insert(tmp, tmp1, 12);//插入表名
	printf("输入插入的数据内容：");
	cin.get();
	cin.get(tmp1, 100);
	x = strlen(tmp) - 1;
	insert(tmp, tmp1, x);
	cout << tmp << endl;
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else{
		printf("插入成功!!!\n");
	}
	system("pause");
	system("cls");
}
////////////删除数据//////////////
void del(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int t = 0;
	show2(con);
	cout << "输入要删除数据的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);
	sprintf_s(tmp, "delete from  where ");
	insert(tmp, tmp1, 12);//插入表名
	printf("输入要删除数据内容的限定条件：");
	cin.get();
	cin.get(tmp1, 100);
	x = strlen(tmp);
	insert(tmp, tmp1, x);
	cout << tmp << endl;
	rt = mysql_real_query(con, tmp, strlen(tmp));//执行delete语句
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else
	{
		printf("删除成功!!!\n");
	}
	system("pause");
	system("cls");
}
////////////修改数据//////////////
void update(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int t = 0;
	show2(con);
	cout << "输入要修改数据的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);
	sprintf_s(tmp, "update  set  where ");
	insert(tmp, tmp1, 7);//插入表名
	printf("输入要更新的数据内容：");
	cin.get();
	cin.get(tmp1, 100);
	insert(tmp, tmp1, strlen(tmp) - 7);
	printf("输入要更新的限定条件（若无限定条件则输入空格）：");
	cin.get();
	cin.get(tmp1, 100);
	x = strlen(tmp);
	insert(tmp, tmp1, x);
	if (tmp1[0] != ' '){
		rt = mysql_real_query(con, tmp, strlen(tmp));
	}
	else{
		rt = mysql_real_query(con, tmp, strlen(tmp) - 7);
	}
	if (rt){
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else{
		printf("更新成功!!!\n");
	}
	system("pause");
	system("cls");
}
////////////删除数据表//////////////
void drop(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int count = 0;
	int t = 0;
	show2(con);
	cout << "输入要删除的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);
	sprintf_s(tmp, "drop table ");
	insert(tmp, tmp1, 11);
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else
	{
		printf("删除成功!!!\n");
	}
	system("pause");
	system("cls");
}
////////////添加索引//////////////
void addindex(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	char tmp2[400];
	int x, rt;
	int t = 0;
	show2(con);
	cout << "输入要添加索引的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);

	cout << "1:普通索引\n";
	cout << "2:唯一索引\n";
	cout << "输入要选择的索引类型的序号：";
	cin >> x;
	system("cls");
	if (x == 1) sprintf_s(tmp, "alter table  add index ()");
	else if (x == 2) sprintf_s(tmp, "alter table  add unique ()");
	insert(tmp, tmp1, 12);//插入表名
	printf("输入添加的索引的名字：");
	cin.get();
	cin.get(tmp2, 100);
	insert(tmp, tmp2, strlen(tmp) - 2);//插入索引名
	system("cls");
	sprintf_s(tmp2, "select column_name from information_schema.columns where table_name = ''");
	insert(tmp2, tmp1, strlen(tmp2) - 1);//表名插入tmp2的语句中
	rt = mysql_real_query(con, tmp2, strlen(tmp2));//执行select column_name的SQL语句
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中
	x = 0;
	while (row = mysql_fetch_row(res)) {
		for (int t = 0; t < mysql_num_fields(res); t++) {
			x++;
			cout << x << ".";
			printf("%s  ", row[t]);
		}
		cout << endl;
	}//输出所有列的列名
	cout << "输入要添加索引的列的序号：";
	cin >> x;
	system("cls");
	rt = mysql_real_query(con, tmp2, strlen(tmp2));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	t = 0;
	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}//找到指定列的列名
	sprintf_s(tmp1, row[0]);
	insert(tmp, tmp1, strlen(tmp) - 1);
	rt = mysql_real_query(con, tmp, strlen(tmp));//执行添加索引的SQL语句
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	else  printf("添加成功！\n");
	system("pause");
	system("cls");
}
////////////显示索引//////////////
void showindex(MYSQL * con)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char tmp[400];
	char tmp1[400];
	int x = 0, rt;
	int t = 0;
	show2(con);
	cout << "输入要显示索引的表的序号：";
	cin >> x;
	system("cls");
	sprintf_s(tmp, "show tables");
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}
	res = mysql_store_result(con);//将结果保存在res结构体中

	while (row = mysql_fetch_row(res)) {
		t++;
		if (t == x) break;
	}
	sprintf_s(tmp1, row[0]);
	sprintf_s(tmp, "show index from ;");
	insert(tmp, tmp1, 16);
	rt = mysql_real_query(con, tmp, strlen(tmp));
	if (rt)
	{
		printf("错误为: %s !!!\n", mysql_error(con));
	}

	output(con);
	mysql_free_result(res);
	system("pause");
	system("cls");
}
////////////对索引操作//////////////
void index(MYSQL * con)
{
	int x;
	cout << "1.显示索引\n";
	cout << "2:添加索引\n";
	cout << "输入要选择的序号：";
	cin >> x;
	cout << endl;
	if (x == 1) showindex(con);
	else if (x == 2) addindex(con);
	system("pause");
	system("cls");
}



int main()
{
	MYSQL * con; //= mysql_init((MYSQL*) 0); 
	char dbuser[30] = "root";
	char dbpasswd[30] = "system";
	char dbip[30] = "localhost";
	char dbname[50] = "mysql";

	char tablename[50] = "book";
	const char *query = NULL;

	int x;
	int y;
	int rt;//return value  

	int count = 0;

	con = mysql_init((MYSQL*)0);
	////////////连接MySQL数据库//////////////
	if (con != NULL && mysql_real_connect(con, dbip, dbuser, dbpasswd, dbname, 3306, NULL, 0)) {
		if (!mysql_select_db(con, dbname)) {
			printf("连接数据库成功!\n");
			con->reconnect = 1;
			query = "set names \'GBK\'";
			rt = mysql_real_query(con, query, strlen(query));
			if (rt) {
				printf("Error making query: %s !!!\n", mysql_error(con));
			}
			else {
				printf("query %s 成功!\n", query);
			}
		}
	}
	else {
		MessageBoxA(NULL, "连接数据库失败，请检查配置!", "", NULL);
	}
	mysql_autocommit(con, 0);

	system("pause");
	system("cls");

	int finish = 0;
	int choice;
	////////////菜单//////////////
	while (!finish) {
		cout << "\n                ********************************Menu********************************\n";
		cout << "                *                        1:新建数据表\n";
		cout << "                *                        2:显示数据表\n";
		cout << "                *                        3:显示数据表内容\n";
		cout << "                *                        4:插入数据\n";
		cout << "                *                        5:删除数据\n";
		cout << "                *                        6:修改数据\n";
		cout << "                *                        7:删除数据表\n";
		cout << "                *                        8:提交当前事务\n";
		cout << "                *                        9:事务回滚\n";
		cout << "                *                       10:索引\n";
		cout << "                *                       11:多用户连接操作\n";
		cout << "                *                       12:退出\n\n\n";
		cout << "                                           请选择选项序号：";
		cin >> choice;
		system("cls");
		switch (choice)
		{
		case 1:
			////////////创建数据表//////////////
			create(con);
			break;

		case 2:
			////////////显示数据表//////////////
			show2(con);
			system("pause");
			system("cls");
			break;

		case 3:
			////////////显示数据表内容//////////////
			show1(con);
			break;

		case 4:
			////////////插入数据//////////////
			insert1(con);
			break;

		case 5:
			////////////删除数据//////////////
			del(con);
			break;

		case 6:
			////////////修改数据//////////////
			update(con);
			break;

		case 7:
			////////////删除数据表//////////////
			drop(con);
			break;

		case 8:
	       ////////////提交事务//////////////
			rt = mysql_commit(con);
			if (rt != 0) {
				printf("提交失败！\n");
			}
			else {
				printf("提交成功!\n");
			}
			system("pause");
			system("cls");
			break;

		case 9:
			////////////事务回滚//////////////
			rt = mysql_rollback(con);
			if (rt != 0) {
				printf("回滚失败！\n");
			}
			else {
				printf("回滚成功!\n");
			}
			system("pause");
			system("cls");
			break;
		   
		case 10:
			////////////索引操作//////////////
			index(con);
			break;

		case 11:
		{
			////////////多用户访问连接//////////////
			thread task01(user01);
			thread task02(user02);
			task01.detach();
			task02.detach();
			system("pause");
			system("cls");
		}
		break;

		case 12:
			////////////退出//////////////
			finish = 1;
			break;
		}
	}
	mysql_close(con);
	mysql_library_end();
	system("pause");
	return 0;
}
#pragma once

class FOBase
{
public:
	virtual void test(void) {}
};
class FOChild final :public FOBase
{
public:
	void test(void) override final {}
};
////���²�����������
//class GrandChild :public Child	
////����Child��final���Σ����ɱ��̳�
//{
//public:
//	void test(void) {}	
//	//Child����test������final���Σ����ɱ��̳�
//};

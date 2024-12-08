#pragma once

//ί�й��캯��-----start
class ConstructorTest
{
public:
    ConstructorTest() {};
    ConstructorTest(int max):m_max(max > 0 ? max : 100)
    {
    }

    //ʹ��ί�й��캯���󣬽���ʼ������ת�Ƹ���һ�����캯��
    //����ֻ���ں���������и�ֵ����
    ConstructorTest(int max, int min): ConstructorTest(max)
    {
        m_min = min > 0 && min < m_max ? min : 1;
    }

    ConstructorTest(int max, int min, int mid) :ConstructorTest(max, min)
    {
        this->m_middle = mid < max && mid > min ? mid : 50;
    }

private:
    int m_min;
    int m_max;
    int m_middle;
};
//ί�й��캯��-----end



//�̳й��캯��-----start
class ConBase
{
public:
    ConBase(int i) :m_i(i) {}
    ConBase(int i, double j) :m_i(i), m_j(j) {}
    ConBase(int i, double j, string k) :m_i(i), m_j(j), m_k(k) {}

    void func() { cout << "ConBase::func()" << endl; }
    void func(int) { cout << "ConBase::func(int)" << endl; }

//private:
    int m_i;
    double m_j;
    string m_k;
};
class ConChild : public ConBase
{
public:
    using ConBase::ConBase; 
    using ConBase::func;    
    //����ʡ�����²��ִ���
    /*Child(int i) :ConBase(i) {}
    Child(int i, double j) :ConBase(i, j) {}
    Child(int i, double j, string k) :ConBase(i, j, k) {}*/

    void func(int, double) { cout << "ConChild::func(int,double)" << endl; }
};

void constructor_test(void);
//�̳й��캯��-----end
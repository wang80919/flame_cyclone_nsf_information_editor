#pragma once
#include <wtypesbase.h>
#include <tchar.h>
#include <vector>
#include <string>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class BTreeNode
{
public:
    BTreeNode()
        :
        m_leftChild(nullptr),
        m_rightChild(nullptr)
    {

    }

    ~BTreeNode()
    {
        Delete(m_leftChild);
        Delete(m_rightChild);
    }

    void Delete(BTreeNode* pNode)
    {
        if (nullptr != pNode)
        {
            Delete(pNode->m_leftChild);
            pNode->m_leftChild = nullptr;
            Delete(pNode->m_rightChild);
            pNode->m_rightChild = nullptr;
            delete pNode;
        }
    }

public:
    BTreeNode* m_leftChild;
    BTreeNode* m_rightChild;
    _tstring m_strValue;
};

class CCalculateUtils
{
public:

    //
    // @brief: 计算字符串算术表达式的结果
    // @param: strExp               表达式, 如: "(2+3)*8-67/2*(4-3)+5"
    // @param: lfResult             计算结果
    // @ret: bool                   操作成功与否
    static bool CCalculate(const _tstring& strExp, double& lfResult);

private:

    //
    // @brief: 获取分解后的表达式元素
    // @param: lpExpBegin           表达式起始位置
    // @param: lpExpEnd             表达式结束位置
    // @param: outExp               分解后的表达式元素序列
    // @ret: bool                   操作成功与否
    static bool GetExpressionElements(LPCTSTR lpExpBegin, LPCTSTR lpExpEnd, std::vector<_tstring>& outExp);

    //
    // @brief: 构建表达式二叉树
    // @param: outExp               表达式元素序列
    // @ret: BTreeNode*             表达式二叉树根结点
    static BTreeNode* BuildBinaryTree(std::vector<_tstring>& outExp);

    //
    // @brief: 计算表达式二叉树
    // @param: pNode                表达式二叉树根结点
    // @ret: double                 计算结果
    static double CalcBinaryTree(const BTreeNode* pNode);

    //
    // @brief: 检查字符串是否为数字
    // @param: strItem              字符串
    // @ret: bool                   是否为数字
    static bool IsNumber(const _tstring& strItem);

    //
    // @brief: 检查字符串是否为运算符
    // @param: strItem              字符串
    // @ret: bool                   是否为运算符
    static bool IsOperator(const _tstring& strItem);

    //
    // @brief: 检查字符是否为运算符
    // @param: ch                   字符
    // @ret: bool                   是否为运算符
    static bool IsOperator(TCHAR ch);

    //
    // @brief: 比较运算符优先级
    // @param: strLeft              运算符1
    // @param: strRight             运算符2
    // @ret: int                    0: 优先级相等, >0: strLeft 优先级高于 strRight, <0: strLeft 优先级低于 strRight
    static int CompareOperator(const _tstring& strLeft, const _tstring& strRight);
};

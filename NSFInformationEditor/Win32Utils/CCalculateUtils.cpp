#include "CCalculateUtils.h"
#include <stack>
#include <map>
#include <queue>
#include <ctgmath>

//运算符优先级
std::map<_tstring, int> g_operatorLevel =
{
    {_T("^"), 2},
    {_T("*"), 1},
    {_T("/"), 1},
    {_T("%"), 1},
    {_T("+"), 0},
    {_T("-"), 0}
};

int CCalculateUtils::CompareOperator(const _tstring& strLeft, const _tstring& strRight)
{
    return g_operatorLevel.find(strLeft)->second - g_operatorLevel.find(strRight)->second;
}

bool CCalculateUtils::IsNumber(const _tstring& strItem)
{
    LPCTSTR lpBegin = strItem.c_str();
    LPCTSTR lpEnd = strItem.c_str();

    if (_istdigit(*lpBegin))
    {
        return true;
    }

    (void)_tcstod(lpBegin, (LPTSTR*)&lpEnd);
    if (lpBegin != lpEnd)
    {
        return true;
    }

    return false;
}

bool CCalculateUtils::IsOperator(TCHAR ch)
{
    if (_T('+') == ch ||
        _T('-') == ch ||
        _T('*') == ch ||
        _T('/') == ch ||
        _T('%') == ch ||
        _T('^') == ch
        )
    {
        return true;
    }

    return false;
}

bool CCalculateUtils::IsOperator(const _tstring& strItem)
{
    if (_T("+") == strItem ||
        _T("-") == strItem ||
        _T("*") == strItem ||
        _T("/") == strItem ||
        _T("%") == strItem ||
        _T("^") == strItem
        )
    {
        return true;
    }

    return false;
}

BTreeNode* CCalculateUtils::BuildBinaryTree(std::vector<_tstring>& outExp)
{
    std::stack<_tstring> opStack;
    std::queue<_tstring> reversePolish;
    bool bSuccess = true;

    for (const auto& item : outExp)
    {
        if (IsNumber(item))
        {
            reversePolish.push(item);
        }
        else if (
            _T("+") == item ||
            _T("-") == item ||
            _T("*") == item ||
            _T("/") == item ||
            _T("%") == item ||
            _T("^") == item ||
            _T("(") == item ||
            _T(")") == item
            )
        {
            _tstring strOperate = item;

            if (_T("(") == item)
            {
                opStack.push(item);
            }
            else if (_T(")") == item)
            {
                while (!opStack.empty())
                {
                    _tstring strOperateTop = opStack.top();
                    opStack.pop();
                    if (_T("(") == strOperateTop)
                    {
                        break;
                    }
                    else
                    {
                        reversePolish.push(strOperateTop);
                    }
                }
            }
            else
            {
                while (!opStack.empty())
                {
                    _tstring strTopOperate = opStack.top();
                    if (_T("(") == strTopOperate)
                    {
                        opStack.push(strOperate);
                        break;
                    }
                    else if (CompareOperator(strTopOperate, strOperate) >= 0)
                    {
                        reversePolish.push(strTopOperate);
                        opStack.pop();
                    }
                    else if (CompareOperator(strTopOperate, strOperate) < 0)
                    {
                        opStack.push(strOperate);
                        break;
                    }
                }

                if (opStack.empty())
                {
                    opStack.push(strOperate);
                }
            }
        }
        else
        {
            return nullptr;
        }
    }

    //剩下的操作符入队
    while (!opStack.empty())
    {
        _tstring strTopOperate = opStack.top();
        reversePolish.push(strTopOperate);
        opStack.pop();
    }

    //将后缀表达式转化成二叉树
    std::stack<BTreeNode*> nodeStack;
    while (!reversePolish.empty())
    {
        _tstring strTopOperate = reversePolish.front();
        BTreeNode* pNode = new (std::nothrow) BTreeNode();
        if (nullptr == pNode)
        {
            bSuccess = false;
            break;
        }

        pNode->m_strValue = strTopOperate;

        if (IsNumber(strTopOperate))
        {
            nodeStack.push(pNode);
        }
        else
        {
            if (nodeStack.size() < 2)
            {
                bSuccess = false;
                break;
            }

            pNode->m_rightChild = nodeStack.top();
            nodeStack.pop();
            pNode->m_leftChild = nodeStack.top();
            nodeStack.pop();
            nodeStack.push(pNode);
        }
        reversePolish.pop();
    }

    if (nodeStack.empty())
    {
        return nullptr;
    }

    if (!bSuccess)
    {
        BTreeNode* pNode = nodeStack.top();
        delete pNode;
        pNode = nullptr;
        return nullptr;
    }

    return nodeStack.top();
}

bool CCalculateUtils::GetExpressionElements(LPCTSTR lpExpBegin, LPCTSTR lpExpEnd, std::vector<_tstring>& outExp)
{
    LPCTSTR lpBegin = lpExpBegin;

    //拆分算式元素
    std::vector<_tstring> vectorExpression;
    while (lpBegin < lpExpEnd)
    {
        //是运算符
        if (IsOperator(*lpBegin))
        {
            bool bLastOperator = false;
            bool bLastNumber = false;
            bool bNumber = IsNumber(lpBegin);
            if (!vectorExpression.empty() && IsOperator(vectorExpression.back()))
            {
                bLastOperator = true;
            }

            if (!vectorExpression.empty() && IsNumber(vectorExpression.back()))
            {
                bLastNumber = true;
            }

            //当前元素可以转为数字
            if (bNumber)
            {
                //最近元素是运算符或者表达式容器为空, 则当作操作数处理
                if (bLastOperator || vectorExpression.empty())
                {
                    LPCTSTR lpTemp = lpBegin;
                    (void)_tcstod(lpBegin, (LPTSTR*)&lpTemp);
                    _tstring strExp = lpBegin;
                    strExp.resize(lpTemp - lpBegin);
                    vectorExpression.push_back(strExp);
                    lpBegin = lpTemp;
                }
                //否则仅当作运算符处理
                else
                {
                    _tstring strExp = lpBegin;
                    strExp.resize(1);
                    vectorExpression.push_back(strExp);
                    lpBegin++;
                }
            }
            //当前元素是运算符
            else
            {
                //最近元素是数字或者表达式元素不为空, 则当作运算符处理
                if (bLastNumber || !vectorExpression.empty())
                {
                    _tstring strExp = lpBegin;
                    strExp.resize(1);
                    vectorExpression.push_back(strExp);
                    lpBegin++;
                }
                //其他情况认为不合法
                else
                {
                    return false;
                }
            }
        }
        //遇到操作数
        else if (IsNumber(lpBegin))
        {
            //遇到右括号与数字挨着, 则添加乘法运算符
            if (!vectorExpression.empty() && _T(")") == vectorExpression.back())
            {
                vectorExpression.push_back(_T("*"));
            }

            LPCTSTR lpTemp = lpBegin;
            (void)_tcstod(lpBegin, (LPTSTR*)&lpTemp);
            _tstring strExp = lpBegin;
            strExp.resize(lpTemp - lpBegin);
            vectorExpression.push_back(strExp);
            lpBegin = lpTemp;
        }
        //遇到括号优先级
        else if (_T('(') == *lpBegin)
        {
            LPCTSTR lpTemp = lpBegin + 1;
            int nDeep = 1;

            while (lpTemp < lpExpEnd)
            {
                if (_T('(') == *lpTemp) nDeep++;
                if (_T(')') == *lpTemp) nDeep--;
                if (0 == nDeep) break;
                lpTemp++;
            }

            //不允许括号不匹配
            if (0 != nDeep)
            {
                return false;
            }

            //不允许空括号()
            if (1 == (lpTemp - lpBegin))
            {
                return false;
            }

            //获取子表达式元素
            std::vector<_tstring> subExp;
            if (!GetExpressionElements(lpBegin + 1, lpTemp, subExp))
            {
                return false;
            }

            //遇到数字与左括号挨着, 则添加乘法运算符
            if (!vectorExpression.empty() && IsNumber(vectorExpression.back()))
            {
                vectorExpression.push_back(_T("*"));
            }

            vectorExpression.push_back(_T("("));
            for (const auto& item : subExp)
            {
                vectorExpression.push_back(item);
            }
            vectorExpression.push_back(_T(")"));

            lpBegin = lpTemp + 1;
        }
        else
        {
            return false;
        }
    }

    if (!vectorExpression.empty())
    {
        //末尾元素是运算符, 但是没有操作数, 则补全一下
        _tstring strOperator = vectorExpression.back();
        if (_T("*") == strOperator || _T("/") == strOperator)
        {
            vectorExpression.push_back(_T("1"));
        }
        else if (_T("+") == strOperator || _T("-") == strOperator)
        {
            vectorExpression.push_back(_T("0"));
        }
        else if (_T("^") == strOperator)
        {
            vectorExpression.push_back(_T("2"));
        }
    }

    outExp = vectorExpression;
    return true;
}

bool CCalculateUtils::CCalculate(const _tstring& strExp, double& lfResult)
{
    //去除空格
    _tstring strDest = strExp;
    _tstring strFind = _T(" ");
    size_t nFind = 0;

    //查找去除空格
    while (_tstring::npos != (nFind = strDest.find(strFind)))
    {
        strDest.replace(nFind, strFind.size(), _T(""));
    };

    if (strDest.empty())
    {
        return false;
    }

    //获取表达式元素
    LPCTSTR lpBegin = strDest.c_str();
    LPCTSTR lpEnd = lpBegin + strDest.size();
    std::vector<_tstring> outExp;
    if (!GetExpressionElements(strDest.c_str(), lpEnd, outExp))
    {
        return false;
    }

    //构建计算二叉树
    BTreeNode* pCalcTree = BuildBinaryTree(outExp);
    if (pCalcTree)
    {
        //计算结果
        lfResult = CalcBinaryTree(pCalcTree);
        delete pCalcTree;
        pCalcTree = nullptr;
    }

    return true;
}

double CCalculateUtils::CalcBinaryTree(const BTreeNode* pNode)
{
    double lfLeftt = 0.0f;
    double lfRight = 0.0f;

    //左右子结点为空, 则返回数字
    if (nullptr == pNode->m_leftChild && nullptr == pNode->m_rightChild)
    {
        return _tcstod(pNode->m_strValue.c_str(), nullptr);
    }

    //计算左结点结果
    if (nullptr != pNode->m_leftChild)
    {
        lfLeftt = CalcBinaryTree(pNode->m_leftChild);
    }

    //计算右结点结果
    if (nullptr != pNode->m_rightChild)
    {
        lfRight = CalcBinaryTree(pNode->m_rightChild);
    }

    if (_T("+") == pNode->m_strValue)
    {
        lfLeftt = lfLeftt + lfRight;
    }

    if (_T("-") == pNode->m_strValue)
    {
        lfLeftt = lfLeftt - lfRight;
    }

    if (_T("*") == pNode->m_strValue)
    {
        lfLeftt = lfLeftt * lfRight;
    }

    if (_T("/") == pNode->m_strValue)
    {
        lfLeftt = lfLeftt / lfRight;
    }

    if (_T("%") == pNode->m_strValue)
    {
        if ((lfLeftt * lfRight) < 0)
        {
            double value = ::floor(lfLeftt / lfRight);
            lfLeftt = lfLeftt - (value * lfRight);
        }
        else
        {
            lfLeftt = ::fmod(lfLeftt, lfRight);
        }
    }

    if (_T("^") == pNode->m_strValue)
    {
        lfLeftt = ::pow(lfLeftt, lfRight);
    }

    return lfLeftt;
}
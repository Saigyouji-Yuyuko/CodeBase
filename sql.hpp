#pragma once

#include "error.hpp"

#include <vector>
#include <string>
#include <memory>
namespace CodeBase 
{
    class RowView
    {
        
    };

    class ExprTree
    {
    public:
        virtual ~ExprTree() = default;
        virtual std::string ToString() const = 0;
        virtual RowView next() = 0;
    };

    class SqlType
    {

    };

    class subExpr;
    
   
    class CreateTableExpr : public ExprTree
    {
        public:
            CreateTableExpr() {}
            std::string ToString() const override;
    };
    class InsertExpr : public ExprTree
    {
        public:
            InsertExpr() {}
            std::string ToString() const override;
    };
    class DeleteExpr : public ExprTree
    {
        public:
            DeleteExpr() {}
            std::string ToString() const override;
    };
    class UpdateExpr : public ExprTree
    {
        public:
            UpdateExpr() {}
            std::string ToString() const override;
    };
    class DropTableExpr : public ExprTree
    {
        public:
            DropTableExpr() {}
            std::string ToString() const override;
    };
    class ShowTablesExpr : public ExprTree
    {
        public:
            ShowTablesExpr() {}
            std::string ToString() const override;
    };  
    ErrorCode Parse2Code(const std::string& expr,std::unique_ptr<ExprTree>& result);

} 
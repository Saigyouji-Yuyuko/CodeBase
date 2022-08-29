#include "sql.hpp"
#include "error.hpp"
#include "sql.hpp"
#include "tokenizer.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <string_view>
#include <algorithm>
#include <cassert>
namespace CodeBase
{
    //select * from tablexxx;
    //create TABEL tablexxx(id int,name varchar(20));
    //insert into tablexxx(id,name) values(1,'abc');
    //delete from tablexxx where id=1;
    //update tablexxx set name='abc' where id=1;
    //drop table tablexxx;
    //show tables;
    //https://forcedotcom.github.io/phoenix/index.html#order
    bool is_keyword( std::string_view str)
    {
        static const std::vector<std::string_view> keywords = {
            "select", "from", "where", "order", "by", "group", "having", "limit", "offset", "desc", "asc", "create", "table", "insert", "into", "values", "delete", "update", "drop", "show", "tables"
        };
        return std::find(keywords.begin(), keywords.end(), str) != keywords.end();
    }
    bool is_operator(std::string_view str)
    {
        static const std::vector<std::string_view> operators = {
            "=", ">", "<", ">=", "<=", "!=", "like", "and", "or", "not", "in", "between", "is", "null", "exists", "union", "intersect", "except"
        };
        return std::find(operators.begin(), operators.end(), str) != operators.end();
    }
    bool is_separator(std::string_view str)
    {
        static const std::vector<std::string_view> separators = {
            ",", ";", "(", ")", ":", "=", ">", "<", ">=", "<=", "!=", "like", "and", "or", "not", "in", "between", "is", "null", "exists", "union", "intersect", "except"
        };
        return std::find(separators.begin(), separators.end(), str) != separators.end();
    }
    
   
    class SQLType;

   
    class SelectItemExpr
    {

        std::string ascol;
    };
    class SelectExpr : public ExprTree
    {
        public: 
            SelectExpr() {}
            std::string ToString() const override;
            RowView next()override;
            friend ErrorCode MakeSelectExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result);
        private:
            bool distinct = false;
            std::vector<std::string> fileds;
            std::string table;
            ExprTree *subquery = nullptr;      
    };
    ErrorCode SelectItemExpr(TokenizerStream& stream,std::unique_ptr<SelectExpr>& expr)
    {
        if(stream.EatToken("*")){
            
        }else{

        }
        
        {
            auto [token,err] = stream.getToken();
            if(token != "," && !compareWithoutCaseSensitive(token,"from")){
                return ErrorCode::SyntaxError();
            }
        }
        return ErrorCode::OK();
    }
    ErrorCode MakeSelectExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result)
    {
        {
            auto ok = stream.EatTokenWithoutCaseSensitive("select");
            assert(ok);
        }
        auto ptr = std::make_unique<SelectExpr>();
        if(!stream.EatTokenWithoutCaseSensitive("all") && 
            stream.EatTokenWithoutCaseSensitive("distinct"))
            ptr->distinct = true;

        do
        {
            auto err = SelectItemExpr(stream,ptr);
            if(err!= ErrorCode::OK()){
                return err;
            }
        }while(stream.EatToken(","));
        auto ok = stream.EatTokenWithoutCaseSensitive("from");
        assert(ok);
                
        
        
    }

    ErrorCode CreateExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result)
    {

    }
    ErrorCode InsertExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result)
    {

    }
    ErrorCode DeleteExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result);
    ErrorCode UpdateExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result);
    ErrorCode DropExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result);
    ErrorCode ShowExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result);

    ErrorCode MakeExpr(TokenizerStream& stream,std::unique_ptr<ExprTree>& result){
        auto [token,err] = stream.previewToken();
        if(err != ErrorCode::OK()){
            return err;
        }
        if (compareWithoutCaseSensitive(token,"select"))
            return MakeSelectExpr(stream,result);
        else if (compareWithoutCaseSensitive(token,"create"))
            return CreateExpr(stream,result);
        else if (compareWithoutCaseSensitive(token,"insert"))
            return InsertExpr(stream,result);
        else if (compareWithoutCaseSensitive(token,"delete"))
            return DeleteExpr(stream,result);
        else if (compareWithoutCaseSensitive(token,"update"))
            return UpdateExpr(stream,result);
        else if (compareWithoutCaseSensitive(token, "drop"))
            return DropExpr(stream,result);
        else if (compareWithoutCaseSensitive(token, "show"))
            return ShowExpr(stream,result);
        else
            return ErrorCode::SyntaxError();
    }

    ErrorCode Parse(const std::string& expr,std::unique_ptr<ExprTree>& result)  
    {
        TokenizerStream stream(expr);
        auto err = MakeExpr(stream, result);
        if(err!= ErrorCode::OK()){
            return err;
        }
        auto ok = stream.EatToken(";");
        if (ok){
            result = nullptr;
            err = ErrorCode::SyntaxError();
        }
        return err;
    }
  
}
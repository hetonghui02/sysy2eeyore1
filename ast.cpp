#include "ast.hpp"
#include "parser.tab.hpp"


FunCallAST::FunCallAST(Token* token){
    type = FunCall;
    BaseEntry* tmp = findString(token->strPtr);
    if(!tmp){
        if(Debug) printf("Error in function 'ast.cpp->FunCallAST::FunCallAST(Token* token)', function not declared!\n");
        if(Debug) printf("function name: %s\n",token->strPtr->c_str());
    }
    if(tmp->type != funSym){
        if(Debug) printf("Error in function 'ast.cpp->FunCallAST::FunCallAST(Token* token)', not function name!\n");
        if(Debug) printf("name: %s\n",token->strPtr->c_str());
    }
    sym = tmp;
}
bool FunCallAST::isNumber(){
    if(dynamic_cast<funSymEntry*>(sym)->isVoid) return false;
    return true;
}
bool FunCallAST::checkParaSize(){
    int fSize = dynamic_cast<funSymEntry*>(sym)->fParams.size();
    int rSize = rParams.size();
    return fSize == rSize;
}

static int oneBinCal(BaseAST* firstNode, BaseAST* secNode, int op){
    int x = firstNode->getVal();
    int y = secNode->getVal();
    switch (op)
    {
        case PLUS:
            return x + y;
        case MINUS:
            return x - y;
        case MULTI:
            return x * y;
        case DIVI:{
            if(!y){
                if(Debug) printf("Error in function 'ast.cpp->oneBinCal', divide by zero!\n");
                return -1;
            }
            return x / y;
        }      
        case MOD:{
            if(!y){
                if(Debug) printf("Error in function 'ast.cpp->oneBinCal', divide by zero!\n");
                return -1;
            }
            return x % y;
        }
        case AND:
            return x && y;
        case OR:
            return x || y;
        case EQ:
            return x == y;
        case NEQ:
            return x != y;
        case GREAT:
            return x > y;
        case LESS:
            return x < y;
        case GorEQ:
            return x >= y;
        case LorEQ:
            return x <= y;
        case PLUandA:{
            firstNode->setVal(x+y);
            return x+y;
        }
        case MINandA:{
            firstNode->setVal(x-y);
            return x+y;
        }
        case MULandA:{
            firstNode->setVal(x*y);
            return x+y;
        }
        case DIVandA:{
            if(!y){
                if(Debug) printf("Error in function 'ast.cpp->oneBinCal', divide by zero!\n");
                return -1;
            }
            firstNode->setVal(x/y);
            return x/y;
        }  
        
        default:
            if(Debug) printf("Error in function 'ast.cpp->oneBinCal', unexpected op!\n");
            return -1;
    }
}

BinaryExpAST::BinaryExpAST(Token* token): \
val(token->val),isConst_(true),isCaled_(false), \
firstExp(nullptr),secExp(nullptr){
    type = BinaryExp;
}
BinaryExpAST::BinaryExpAST(BaseAST* a, BaseAST*b, Token* token): \
val(0),isConst_(false),isCaled_(false), \
firstExp(a),secExp(b),op(token->type){
    type = BinaryExp;
}
BinaryExpAST::~BinaryExpAST(){
    if(firstExp) delete firstExp;
    if(secExp) delete secExp;
}
int BinaryExpAST::cal(){
    if(isCaled_ || isConst_) return val;

    firstExp->cal();
    secExp->cal();

    if(firstExp->isConst()&&secExp->isConst()){
        isConst_ = true;
        val = oneBinCal(firstExp, secExp, op);
    }
    isCaled_ = true;
    return val;
}
bool BinaryExpAST::isNumber(){
    if(isConst()) return true;
    if(firstExp && !firstExp->isConst()) return false;
    if(secExp && !secExp->isConst()) return false;
    return true;
}

UnaryExpAST::UnaryExpAST(BaseAST* a, Token* token): \
val(0),op(token->type),firstExp(a), \
isConst_(false),isCaled_(false){
    type = UnaryExp;
}
UnaryExpAST::~UnaryExpAST(){
    if(firstExp) delete firstExp;
}
void UnaryExpAST::dealOp(Token* token){
    switch (op){
        case PLUS://+，op总与token->type相同
            op = token->type;
            break;
        case MINUS://-
            switch (token->type){
                case PLUS:
                    op = MINUS;//-+:-
                    break;
                case MINUS:
                    op = PLUS;//--:+
                    break;
                case NOT:
                    op = Minus_Not;//-!:Minus_Not
                    break;
                default:
                    break;
            }
            break;
        case NOT://!
            switch (token->type){
                case PLUS:
                    op = NOT;//!+:!
                    break;
                case MINUS:
                    op = NOT;//!-:!
                    break;
                case NOT:
                    op = PLUS;//!!:+
                    break;
                default:
                    break;
            }
            break;
        case Minus_Not://"-!"
            switch (token->type){ //Minus_Not: +、-:Minus_Not, !:Minus_Not_Not
                case PLUS:
                    op = Minus_Not;
                    break;
                case MINUS:
                    op = Minus_Not;
                    break;
                case NOT:
                    op = Minus_Not_Not;
                    break;
                default:
                    break;
            }
            break;   
        case Minus_Not_Not://"-!!"
            switch (token->type){ //Minus_Not_Not: +、-:Minus_Not_Not, !:Minus_Not
                case PLUS:
                    op = Minus_Not_Not;
                    break;
                case MINUS:
                    op = Minus_Not_Not;
                    break;
                case NOT:
                    op = Minus_Not;
                    break;
                default:
                    break;
            }
            break;      
        default:
            break;
    }
}
int UnaryExpAST::cal(){
    if(isCaled_ || isConst_) return val;
    if(!firstExp){
        if(Debug) printf("Error in function 'ast.cpp->UnaryExpAST::cal', firstExp is NULL!\n");
    }

    firstExp->cal();

    if(firstExp->isConst()){
        isConst_ = true;
        val = firstExp->getVal();
        switch (op){
            case PLUS:
                break;
            case MINUS:
                val = -val;
                break;
            case NOT:
                val = !val;
                break;
            case Minus_Not://"-!"
                if(val==0) val = -1;
                else val = 0;
                break;
            case Minus_Not_Not://"-!!"
                if(val==0) val = 0;
                else val = -1; 
                break;
            default:
                break;
        }
    }
    isCaled_ = true;
    return val;
}
bool UnaryExpAST::isNumber(){
    if(isConst()) return true;
    if(firstExp && !firstExp->isConst()) return false;
    return true;
}

LvalAST::LvalAST(Token* token, BaseAST* ast): \
isConst_(false),isCaled_(false),isLeft_(false), \
val(0){
    type=Lval;
    BaseEntry* tmp = findString(token->strPtr);
    if(!tmp){
        if(Debug) printf("Error in function 'ast.cpp->LvalAST::LvalAST(Token* token, BaseAST* ast)', variable not declared!\n");
        if(Debug) printf("variable name: %s\n",token->strPtr->c_str());
    }
    if(tmp->type != varSym){
        if(Debug) printf("Error in function 'ast.cpp->LvalAST::LvalAST(Token* token, BaseAST* ast)', not variable name!\n");
        if(Debug) printf("name: %s\n",token->strPtr->c_str());
    }
    sym = tmp;
    indexs = dynamic_cast<arrAST*>(ast)->output();
}
bool LvalAST::checkTooManyIndex(){
    int lvalK = indexs.size();
    int defineK = dynamic_cast<varSymEntry*>(sym)->dims.size();
    return lvalK > defineK-1 ;
}
bool LvalAST::checkNegIndex(){
    int size = indexs.size();
    for(int i=0; i<size; ++i){
        if(!indexs[i]->isConst()) continue;
        if(indexs[i]->getVal()<0){
            if(Debug) printf("Error: negative index! Variable name: %s\n",sym->strp->c_str());
            return true;
        }
    }
    return false;
}
bool LvalAST::checkIfOutOfBound(){
    varSymEntry* tmp = dynamic_cast<varSymEntry*>(sym);
    int size = indexs.size();
    int arrSize = tmp->dims.size(); //dims: 记录数组基址，例如a[3][2],为1,2,6
    for(int i=0; i<size; ++i){
        if(!indexs[i]->isConst()) continue;
        int k = tmp->dims[arrSize-1-i] / tmp->dims[arrSize-2-i];
        if( k && indexs[i]->getVal()>=k ){
            if(Debug) printf("Error: index out of bound! Variable name: %s\n",sym->strp->c_str());
            return true;
        }
    }
    return false;
}
bool LvalAST::isNumber(){
    if(isConst_) return true;
    if(dynamic_cast<varSymEntry *>(sym)->dims.size() == indexs.size()+1 ){
        for(auto i: indexs){
            if(!i->isNumber())
                return false;
            return true;
        }
    }
    return false;
}
int LvalAST::cal(){
    if(isCaled_ || isConst_) return val;
    if( !dynamic_cast<varSymEntry *>(sym)->isConst ) return val;
    for( auto i:indexs ) if(!i->isConst()) return val;
    isConst_ = true, isCaled_ = true;
    vector<int> tmp;
    for( auto i:indexs ) tmp.push_back(i->getVal());
    int i = getArrAddr(dynamic_cast<varSymEntry*>(sym)->dims, tmp);
    BaseAST* tmp2 = dynamic_cast<DefineAST*>(sym->father)->getInitVal(i);
    if(tmp2) val = tmp2->getVal();
    return val;
}

BaseAST* DefineAST::getInitVal(int index){
    auto tmp = mmap.find(index);
    if( tmp==mmap.end() ) return nullptr;
    return tmp->second;
}

map<int, BaseAST* > ListTree::Flatten(const vector<int> &dims){
	map<int, BaseAST* > ret; 
	vector<int> savedim = dims; // Copy constructor
	_Flatten_(savedim, ret, 0, savedim.back());
	return ret;
}

void ListTree::_Flatten_(vector<int> &dim, map<int, BaseAST* > &res, int ind, int upper){
	if( !astptr && sublist.empty() ) return;
	if(dim.size() == 1) {
		if(ind >= upper) return;
		if(astptr) res[ind] = astptr;
		return;
	}

	const int tot = dim.back();
	dim.pop_back();
	const int no = dim.back();

	for(auto p: sublist){
		if( !dynamic_cast<ListTree*>(p)->astptr ){
			dynamic_cast<ListTree*>(p)->_Flatten_(dim, res, ind, upper);
			ind += no;
			if(ind >= upper) return;
		}
		else {
			if(ind >= upper) return;
			res[ind] = dynamic_cast<ListTree*>(p)->astptr;
			ind += 1;
		}
	}
	dim.push_back(tot);
	return;
}

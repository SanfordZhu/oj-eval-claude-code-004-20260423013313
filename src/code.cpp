#include <bits/stdc++.h>
using namespace std;

namespace util {
    string trim(const string &s) {
        size_t l = s.find_first_not_of(' ');
        if (l == string::npos) return "";
        size_t r = s.find_last_not_of(' ');
        return s.substr(l, r - l + 1);
    }
    vector<string> split_spaces(const string &s) {
        vector<string> t; size_t i=0,n=s.size();
        while(i<n){ while(i<n && s[i]==' ') i++; if(i>=n) break; size_t j=i; while(j<n && s[j]!=' ') j++; t.emplace_back(s.substr(i,j-i)); i=j; }
        return t;
    }
    bool isPrintable(char c){ return c >= 32 && c <= 126; }
}

struct StackEntry { string uid; int priv; string selected_isbn; };

struct BookRec { string isbn,name,author,keyword; double price; int stock; bool deleted; };
struct BookStore {
    string path = "books.txt";
    BookStore(){ ifstream fin(path); if(!fin.good()){ ofstream fout(path); fout.close(); } }
    static optional<BookRec> parseLine(const string &line){
        // isbn\tname\tauthor\tkeyword\tprice\tstock\tdeleted
        vector<string> p; size_t i=0; for(int k=0;k<6;k++){ auto q=line.find('\t', i); if(q==string::npos) return nullopt; p.push_back(line.substr(i,q-i)); i=q+1; } p.push_back(line.substr(i)); if(p.size()!=7) return nullopt;
        BookRec b; b.isbn=p[0]; b.name=p[1]; b.author=p[2]; b.keyword=p[3]; b.price=0.0; try{ b.price=stod(p[4]); }catch(...){ b.price=0.0; } b.stock=0; try{ b.stock=stoi(p[5]); }catch(...){ b.stock=0; } b.deleted=(p[6]=="1"); return b;
    }
    bool exists(const string &isbn){ ifstream fin(path); string line; while(getline(fin,line)){ auto b=parseLine(line); if(b && !b->deleted && b->isbn==isbn) return true; } return false; }
    optional<BookRec> get(const string &isbn){ ifstream fin(path); string line; while(getline(fin,line)){ auto b=parseLine(line); if(b && !b->deleted && b->isbn==isbn) return b; } return nullopt; }
    void writeAll(const vector<string> &lines){ ofstream fout(path, ios::trunc); for(auto &l: lines) fout<<l<<"\n"; }
    bool upsert(const BookRec &nb){ ifstream fin(path); vector<string> lines; string line; bool done=false; while(getline(fin,line)){ auto b=parseLine(line); if(b && b->isbn==nb.isbn){ string nl = nb.isbn+'\t'+nb.name+'\t'+nb.author+'\t'+nb.keyword+'\t'+to_string(nb.price)+'\t'+to_string(nb.stock)+'\t'+(nb.deleted?"1":"0"); lines.push_back(nl); done=true; } else lines.push_back(line);} fin.close(); if(!done){ string nl = nb.isbn+'\t'+nb.name+'\t'+nb.author+'\t'+nb.keyword+'\t'+to_string(nb.price)+'\t'+to_string(nb.stock)+'\t'+(nb.deleted?"1":"0"); lines.push_back(nl);} writeAll(lines); return true; }
    bool updateIsbn(const string &oldIsbn, const string &newIsbn){ ifstream fin(path); vector<string> lines; string line; bool ok=false; while(getline(fin,line)){ auto b=parseLine(line); if(b && b->isbn==oldIsbn){ b->isbn=newIsbn; string nl=b->isbn+'\t'+b->name+'\t'+b->author+'\t'+b->keyword+'\t'+to_string(b->price)+'\t'+to_string(b->stock)+'\t'+(b->deleted?"1":"0"); lines.push_back(nl); ok=true; } else lines.push_back(line);} fin.close(); writeAll(lines); return ok; }
    vector<BookRec> listAll(){ vector<BookRec> v; ifstream fin(path); string line; while(getline(fin,line)){ auto b=parseLine(line); if(b && !b->deleted) v.push_back(*b);} return v; }
};

struct FinanceLog {
    string path = "finance.txt";
    FinanceLog(){ ifstream fin(path); if(!fin.good()){ ofstream fout(path); fout.close(); } }
    void addIncome(double x){ ofstream fout(path, ios::app); fout<<fixed<<setprecision(2)<<x<<"\t"<<0<<"\n"; }
    void addExpenditure(double x){ ofstream fout(path, ios::app); fout<<fixed<<setprecision(2)<<0<<"\t"<<x<<"\n"; }
    vector<pair<double,double>> all(){ vector<pair<double,double>> v; ifstream fin(path); string a,b; while(fin>>a>>b){ double ai=stod(a), be=stod(b); v.emplace_back(ai,be);} return v; }
    pair<double,double> sumLast(int cnt){ auto v=all(); if(cnt<0) cnt=0; if(cnt>(int)v.size()) return {nan(""), nan("")}; double inc=0,exp=0; for(int i=max(0,(int)v.size()-cnt); i<(int)v.size(); ++i){ inc+=v[i].first; exp+=v[i].second; } return {inc,exp}; }
};

struct OperationLog {
    string path = "ops.txt";
    OperationLog(){ ifstream fin(path); if(!fin.good()){ ofstream fout(path); fout.close(); } }
    void add(const string &uid, const string &op){ ofstream fout(path, ios::app); fout<<uid<<'\t'<<op<<"\n"; }
    map<string,int> counts(){ map<string,int> m; ifstream fin(path); string uid, op; while(fin>>uid>>op){ m[uid]++; } return m; }
    vector<pair<string,string>> lastN(int n){ vector<pair<string,string>> v; ifstream fin(path); string uid, op; while(fin>>uid>>op){ v.emplace_back(uid,op);} if(n>(int)v.size()) n=v.size(); return vector<pair<string,string>>(v.end()-n, v.end()); }
};

struct AccountStore {
    string path = "accounts.txt";
    AccountStore(){
        ifstream fin(path); if(!fin.good()){ ofstream fout(path); fout.close(); }
        // ensure root exists
        if(!exists("root")) create("root","sjtu",7,"root");
    }
    struct Rec { string uid,pwd,uname; int priv; bool deleted; };
    static optional<Rec> parseLine(const string &line){
        // uid\tpwd\tuname\tpriv\tdeleted
        vector<string> parts; size_t i=0; for(int k=0;k<4;k++){ auto p=line.find('\t', i); if(p==string::npos) return nullopt; parts.push_back(line.substr(i,p-i)); i=p+1; }
        parts.push_back(line.substr(i)); if(parts.size()!=5) return nullopt;
        Rec r{parts[0],parts[1],parts[2],stoi(parts[3]), parts[4]=="1"}; return r;
    }
    bool exists(const string &uid){ ifstream fin(path); string line; while(getline(fin,line)){ auto rec=parseLine(line); if(rec && !rec->deleted && rec->uid==uid) return true; } return false; }
    optional<Rec> get(const string &uid){ ifstream fin(path); string line; while(getline(fin,line)){ auto rec=parseLine(line); if(rec && !rec->deleted && rec->uid==uid) return rec; } return nullopt; }
    bool create(const string &uid,const string &pwd,int priv,const string &uname){ ofstream fout(path, ios::app); fout<<uid<<'\t'<<pwd<<'\t'<<uname<<'\t'<<priv<<'\t'<<0<<"\n"; return true; }
    bool setPassword(const string &uid,const string &newpwd){ ifstream fin(path); vector<string> lines; string line; bool ok=false; while(getline(fin,line)){ auto rec=parseLine(line); if(rec && rec->uid==uid && !rec->deleted){ rec->pwd=newpwd; line=rec->uid+'\t'+rec->pwd+'\t'+rec->uname+'\t'+to_string(rec->priv)+'\t'+(rec->deleted?"1":"0"); ok=true; } lines.push_back(line);} fin.close(); ofstream fout(path, ios::trunc); for(auto &l: lines) fout<<l<<"\n"; return ok; }
    bool remove(const string &uid){ ifstream fin(path); vector<string> lines; string line; bool ok=false; while(getline(fin,line)){ auto rec=parseLine(line); if(rec && rec->uid==uid && !rec->deleted){ rec->deleted=true; line=rec->uid+'\t'+rec->pwd+'\t'+rec->uname+'\t'+to_string(rec->priv)+'\t'+(rec->deleted?"1":"0"); ok=true; } lines.push_back(line);} fin.close(); ofstream fout(path, ios::trunc); for(auto &l: lines) fout<<l<<"\n"; return ok; }
};

static bool valid_id(const string &s){ if(s.size()>30||s.empty()) return false; for(char c:s){ if(!(isalnum((unsigned char)c)||c=='_')) return false; } return true; }
static bool valid_pwd(const string &s){ return valid_id(s); }
static bool valid_username(const string &s){ if(s.size()>30||s.empty()) return false; for(char c:s){ if(!util::isPrintable(c)) return false; } return true; }
static bool valid_priv_str(const string &s){ if(s.size()!=1) return false; if(!isdigit((unsigned char)s[0])) return false; int p=s[0]-'0'; return p==1||p==3||p==7; }

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);

    AccountStore accounts; vector<StackEntry> st;
    auto current_priv = [&](){ return st.empty()?0:st.back().priv; };
    auto current_uid = [&](){ return st.empty()?string(""):st.back().uid; };

    string line;
    while(getline(cin,line)){
        string s = util::trim(line); if(s.empty()) continue; auto tokens = util::split_spaces(s); string cmd = tokens[0];
        if(cmd=="exit"||cmd=="quit") break;
        auto invalid=[&](){ cout<<"Invalid\n"; };

        if(cmd=="su"){
            OperationLog oplog;
            if(tokens.size()!=2 && tokens.size()!=3){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            auto rec = accounts.get(uid); if(!rec){ invalid(); continue; }
            int target_priv = rec->priv;
            if(tokens.size()==2){ if(current_priv()>target_priv){ st.push_back({uid,target_priv,""}); oplog.add(current_uid(), string("su ")+uid); } else { invalid(); } }
            else { string pwd=tokens[2]; if(!valid_pwd(pwd)){ invalid(); continue; } if(rec->pwd==pwd){ st.push_back({uid,target_priv,""}); oplog.add(current_uid(), string("su ")+uid); } else { invalid(); } }
        } else if(cmd=="logout"){
            if(tokens.size()!=1){ invalid(); continue; } if(st.empty()){ invalid(); continue; } OperationLog oplog; oplog.add(current_uid(), "logout"); st.pop_back();
        } else if(cmd=="register"){
            if(tokens.size()!=4){ invalid(); continue; }
            string uid=tokens[1], pwd=tokens[2], uname=tokens[3]; if(!valid_id(uid)||!valid_pwd(pwd)||!valid_username(uname)){ invalid(); continue; }
            if(accounts.exists(uid)){ invalid(); continue; }
            accounts.create(uid,pwd,1,uname); OperationLog oplog; oplog.add(current_uid(), string("register ")+uid);
        } else if(cmd=="passwd"){
            if(tokens.size()!=3 && tokens.size()!=4){ invalid(); continue; }
            if(current_priv()<1){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            auto rec=accounts.get(uid); if(!rec){ invalid(); continue; }
            string newpwd=tokens.back(); if(!valid_pwd(newpwd)){ invalid(); continue; }
            if(tokens.size()==3){ if(current_priv()==7){ accounts.setPassword(uid,newpwd); OperationLog oplog; oplog.add(current_uid(), string("passwd ")+uid); } else { invalid(); continue; } }
            else { string curpwd=tokens[2]; if(!valid_pwd(curpwd)){ invalid(); continue; } if(rec->pwd==curpwd){ accounts.setPassword(uid,newpwd); OperationLog oplog; oplog.add(current_uid(), string("passwd ")+uid); } else { invalid(); continue; } }
        } else if(cmd=="useradd"){
            if(tokens.size()!=5){ invalid(); continue; }
            if(current_priv()<3){ invalid(); continue; }
            string uid=tokens[1], pwd=tokens[2], privs=tokens[3], uname=tokens[4]; if(!valid_id(uid)||!valid_pwd(pwd)||!valid_username(uname)||!valid_priv_str(privs)){ invalid(); continue; }
            int p = privs[0]-'0'; if(p>=current_priv()){ invalid(); continue; } if(accounts.exists(uid)){ invalid(); continue; }
            accounts.create(uid,pwd,p,uname); OperationLog oplog; oplog.add(current_uid(), string("useradd ")+uid);
        } else if(cmd=="delete"){
            if(tokens.size()!=2){ invalid(); continue; }
            if(current_priv()<7){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            if(!accounts.exists(uid)){ invalid(); continue; }
            bool logged=false; for(auto &e: st){ if(e.uid==uid){ logged=true; break; } }
            if(logged){ invalid(); continue; }
            accounts.remove(uid); OperationLog oplog; oplog.add(current_uid(), string("delete ")+uid);
        } else if(cmd=="select"){
            if(tokens.size()!=2){ invalid(); continue; }
            if(current_priv()<3){ invalid(); continue; }
            string isbn=tokens[1]; if(isbn.empty()||isbn.size()>20){ invalid(); continue; }
            if(st.empty()){ invalid(); continue; }
            st.back().selected_isbn = isbn;
            // create book if missing with only ISBN
            BookStore bs; if(!bs.exists(isbn)){ BookRec rec{isbn, "", "", "", 0.0, 0, false}; bs.upsert(rec); }
            OperationLog oplog; oplog.add(current_uid(), string("select ")+isbn);
        } else if(cmd=="modify"){
            if(current_priv()<3){ invalid(); continue; }
            if(st.empty()||st.back().selected_isbn.empty()){ invalid(); continue; }
            string cur = st.back().selected_isbn;
            // parse options, prevent duplicates
            set<string> seen;
            string newIsbn=""; bool ch_isbn=false;
            optional<string> name, author, keyword, price;
            for(size_t i=1;i<tokens.size();++i){ string opt=tokens[i]; auto eq=opt.find('='); if(eq==string::npos){ invalid(); goto nextline; } string key=opt.substr(0,eq); string rhs=opt.substr(eq+1); if(seen.count(key)){ invalid(); goto nextline; } seen.insert(key);
                if(key=="-ISBN"){ if(rhs.empty()){ invalid(); goto nextline; } if(rhs==cur){ invalid(); goto nextline; } if(rhs.size()>20){ invalid(); goto nextline; } ch_isbn=true; newIsbn=rhs; }
                else if(key=="-name"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); goto nextline; } string val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); goto nextline; } name=val; }
                else if(key=="-author"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); goto nextline; } string val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); goto nextline; } author=val; }
                else if(key=="-keyword"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); goto nextline; } string val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); goto nextline; } // check duplicates
                    vector<string> segs; string curk; for(char c: val){ if(c=='|'){ segs.push_back(curk); curk.clear(); } else curk.push_back(c);} segs.push_back(curk); set<string> stp; for(auto &x: segs){ if(x.empty()){ invalid(); goto nextline; } if(stp.count(x)){ invalid(); goto nextline; } stp.insert(x);} keyword=val; }
                else if(key=="-price"){ if(rhs.empty()){ invalid(); goto nextline; } price=rhs; }
                else { invalid(); goto nextline; }
            }
            {
                // create if missing
                BookStore bs; auto b = bs.get(cur); BookRec rec; if(b) rec=*b; else { rec = BookRec{cur, "", "", "", 0.0, 0, false}; bs.upsert(rec);} // now apply
                if(ch_isbn){ if(bs.exists(newIsbn)){ invalid(); goto nextline; } bs.updateIsbn(cur, newIsbn); st.back().selected_isbn = newIsbn; cur = newIsbn; rec.isbn = newIsbn; }
                if(name) rec.name = *name;
                if(author) rec.author = *author;
                if(keyword) rec.keyword = *keyword;
                if(price){ try{ rec.price = stod(*price); }catch(...){ invalid(); goto nextline; } }
                bs.upsert(rec); OperationLog oplog; oplog.add(current_uid(), string("modify ")+cur);
            }
            nextline:;
        } else if(cmd=="import"){
            if(tokens.size()!=3){ invalid(); continue; }
            if(current_priv()<3){ invalid(); continue; }
            if(st.empty()||st.back().selected_isbn.empty()){ invalid(); continue; }
            string cur = st.back().selected_isbn;
            int qty=0; try{ qty = stoi(tokens[1]); }catch(...){ qty=0; }
            double cost=0.0; try{ cost = stod(tokens[2]); }catch(...){ cost=0.0; }
            if(qty<=0 || !(cost>0.0)){ invalid(); continue; }
            BookStore bs; auto b = bs.get(cur); BookRec rec; if(b) rec=*b; else { rec = BookRec{cur, "", "", "", 0.0, 0, false}; }
            rec.stock += qty; bs.upsert(rec);
            FinanceLog fl; fl.addExpenditure(cost);
            OperationLog oplog; oplog.add(current_uid(), string("import ")+to_string(qty));
        } else if(cmd=="show"){
            if(current_priv()<1){ invalid(); continue; }
            BookStore bs; vector<BookRec> all = bs.listAll();
            string type=""; string val=""; if(tokens.size()==2){ string opt=tokens[1]; auto eq=opt.find('='); if(eq==string::npos){ invalid(); continue; } string key=opt.substr(0,eq); string rhs=opt.substr(eq+1);
                if(key=="-ISBN"){ type="isbn"; val=rhs; if(val.empty()||val.size()>20){ invalid(); continue; } }
                else if(key=="-name"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); continue; } val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); continue; } type="name"; }
                else if(key=="-author"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); continue; } val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); continue; } type="author"; }
                else if(key=="-keyword"){ if(rhs.size()<2||rhs.front()!='"'||rhs.back()!='"'){ invalid(); continue; } val=rhs.substr(1,rhs.size()-2); if(val.empty()||val.size()>60){ invalid(); continue; } if(val.find('|')!=string::npos){ invalid(); continue; } type="keyword"; }
                else { invalid(); continue; }
            } else if(tokens.size()>2){ invalid(); continue; }
            vector<BookRec> res; for(auto &b: all){ bool ok=true; if(type=="isbn") ok = (b.isbn==val); else if(type=="name") ok=(b.name==val); else if(type=="author") ok=(b.author==val); else if(type=="keyword") ok=(b.keyword==val); if(ok) res.push_back(b);} sort(res.begin(),res.end(),[](const BookRec&a,const BookRec&b){return a.isbn<b.isbn;});
            if(res.empty()){ cout<<"\n"; continue; }
            for(auto &b: res){ cout<<b.isbn<<'\t'<<b.name<<'\t'<<b.author<<'\t'<<b.keyword<<'\t'<<fixed<<setprecision(2)<<b.price<<'\t'<<b.stock<<"\n"; }
        } else if(cmd=="buy"){
            if(tokens.size()!=3){ invalid(); continue; }
            if(current_priv()<1){ invalid(); continue; }
            string isbn=tokens[1]; int q=0; try{ q=stoi(tokens[2]); }catch(...){ q=0; }
            if(q<=0){ invalid(); continue; }
            BookStore bs; auto b=bs.get(isbn); if(!b){ invalid(); continue; }
            if(b->stock < q){ invalid(); continue; }
            b->stock -= q; bs.upsert(*b);
            double total = b->price * q; FinanceLog fl; fl.addIncome(total); cout<<fixed<<setprecision(2)<<total<<"\n"; OperationLog oplog; oplog.add(current_uid(), string("buy ")+isbn);
        } else if(cmd=="show" && tokens.size()>=2 && tokens[1]=="finance"){
            // handle show finance ([Count])?
            if(current_priv()<7){ invalid(); continue; }
            if(tokens.size()==2){ FinanceLog fl; auto p = fl.sumLast(-1); cout<<"+ "<<fixed<<setprecision(2)<<p.first<<" - "<<fixed<<setprecision(2)<<p.second<<"\n"; }
            else if(tokens.size()==3){ int c=0; try{ c=stoi(tokens[2]); }catch(...){ invalid(); continue; } if(c==0){ cout<<"\n"; continue; } FinanceLog fl; auto p=fl.sumLast(c); if(isnan(p.first)||isnan(p.second)){ invalid(); continue; } cout<<"+ "<<fixed<<setprecision(2)<<p.first<<" - "<<fixed<<setprecision(2)<<p.second<<"\n"; }
            else { invalid(); continue; }
        } else if(cmd=="log"){
            if(current_priv()<7){ invalid(); continue; }
            OperationLog opl; auto last = opl.lastN(50); // arbitrary cap
            for(auto &pr: last){ cout<<pr.first<<"\t"<<pr.second<<"\n"; }
        } else if(cmd=="report"){
            if(current_priv()<7){ invalid(); continue; }
            if(tokens.size()!=2){ invalid(); continue; }
            if(tokens[1]=="finance"){ FinanceLog fl; auto v=fl.all(); double inc=0,exp=0; for(auto &p:v){ inc+=p.first; exp+=p.second; } cout<<"+ "<<fixed<<setprecision(2)<<inc<<" - "<<fixed<<setprecision(2)<<exp<<"\n"; }
            else if(tokens[1]=="employee"){ OperationLog opl; auto m=opl.counts(); for(auto &kv: m){ cout<<kv.first<<"\t"<<kv.second<<"\n"; } }
            else { invalid(); }
        } else {
            // unimplemented commands
            invalid();
        }
    }
    return 0;
}

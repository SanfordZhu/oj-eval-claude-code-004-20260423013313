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

    string line;
    while(getline(cin,line)){
        string s = util::trim(line); if(s.empty()) continue; auto tokens = util::split_spaces(s); string cmd = tokens[0];
        if(cmd=="exit"||cmd=="quit") break;
        auto invalid=[&](){ cout<<"Invalid\n"; };

        if(cmd=="su"){
            if(tokens.size()!=2 && tokens.size()!=3){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            auto rec = accounts.get(uid); if(!rec){ invalid(); continue; }
            int target_priv = rec->priv;
            if(tokens.size()==2){ if(current_priv()>target_priv){ st.push_back({uid,target_priv}); } else { invalid(); } }
            else { string pwd=tokens[2]; if(!valid_pwd(pwd)){ invalid(); continue; } if(rec->pwd==pwd){ st.push_back({uid,target_priv}); } else { invalid(); } }
        } else if(cmd=="logout"){
            if(tokens.size()!=1){ invalid(); continue; } if(st.empty()){ invalid(); continue; } st.pop_back();
        } else if(cmd=="register"){
            if(tokens.size()!=4){ invalid(); continue; }
            string uid=tokens[1], pwd=tokens[2], uname=tokens[3]; if(!valid_id(uid)||!valid_pwd(pwd)||!valid_username(uname)){ invalid(); continue; }
            if(accounts.exists(uid)){ invalid(); continue; }
            accounts.create(uid,pwd,1,uname);
        } else if(cmd=="passwd"){
            if(tokens.size()!=3 && tokens.size()!=4){ invalid(); continue; }
            if(current_priv()<1){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            auto rec=accounts.get(uid); if(!rec){ invalid(); continue; }
            string newpwd=tokens.back(); if(!valid_pwd(newpwd)){ invalid(); continue; }
            if(tokens.size()==3){ if(current_priv()==7){ accounts.setPassword(uid,newpwd); } else { invalid(); continue; } }
            else { string curpwd=tokens[2]; if(!valid_pwd(curpwd)){ invalid(); continue; } if(rec->pwd==curpwd){ accounts.setPassword(uid,newpwd); } else { invalid(); continue; } }
        } else if(cmd=="useradd"){
            if(tokens.size()!=5){ invalid(); continue; }
            if(current_priv()<3){ invalid(); continue; }
            string uid=tokens[1], pwd=tokens[2], privs=tokens[3], uname=tokens[4]; if(!valid_id(uid)||!valid_pwd(pwd)||!valid_username(uname)||!valid_priv_str(privs)){ invalid(); continue; }
            int p = privs[0]-'0'; if(p>=current_priv()){ invalid(); continue; } if(accounts.exists(uid)){ invalid(); continue; }
            accounts.create(uid,pwd,p,uname);
        } else if(cmd=="delete"){
            if(tokens.size()!=2){ invalid(); continue; }
            if(current_priv()<7){ invalid(); continue; }
            string uid=tokens[1]; if(!valid_id(uid)){ invalid(); continue; }
            if(!accounts.exists(uid)){ invalid(); continue; }
            bool logged=false; for(auto &e: st){ if(e.uid==uid){ logged=true; break; } }
            if(logged){ invalid(); continue; }
            accounts.remove(uid);
        } else if(cmd=="select"){
            if(tokens.size()!=2){ invalid(); continue; }
            if(current_priv()<3){ invalid(); continue; }
            string isbn=tokens[1]; if(isbn.empty()||isbn.size()>20){ invalid(); continue; }
            if(st.empty()){ invalid(); continue; }
            st.back().selected_isbn = isbn; // create lazily later in modify/import
        } else {
            // unimplemented commands
            invalid();
        }
    }
    return 0;
}

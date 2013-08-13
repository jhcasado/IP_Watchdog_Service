#ifndef __registry_class_defined__
#define __registry_class_defined__

#include <string>
#include "windows.h"

#ifdef WANT_TRACE
#include "stdio.h"
#define REG_TRACE(f) f;
#else
#define REG_TRACE(f) ;
#endif

struct registry  
{
private:
	const registry & operator=(const registry & r);
	registry(){};
protected:
	HKEY _R;
public:
	enum Key{
		hkey_classes_root=0,
		hkey_current_config=1,
		hkey_current_user=2,
		hkey_local_machine=3,
		hkey_users=4,
		hkey_dyn_data=5
	};
protected:
	void create_key(const char *name, Key key){
		HKEY _key;
		switch(key){
		case hkey_classes_root: _key = HKEY_CLASSES_ROOT; break;
		case hkey_current_config: _key = HKEY_CURRENT_CONFIG; break;
		case hkey_local_machine: _key = HKEY_LOCAL_MACHINE; break;
		case hkey_users: _key = HKEY_USERS; break;
		case hkey_dyn_data: _key = HKEY_DYN_DATA; break;
		case hkey_current_user: 
		default: _key = HKEY_CURRENT_USER; break;
		}

		DWORD disp;
		REG_TRACE(printf("REG_TRACE:\tconstructing registry for \"%s\" ... ",name));
		if(ERROR_SUCCESS!=RegCreateKeyEx(_key,name,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&_R,&disp)){
			REG_TRACE(printf("Failed!\n"));
			_R=0;
		}else{
			REG_TRACE(printf("OK\n"));
		}
		//(disp!=REG_CREATED_NEW_KEY)? existing key: new key;
	}
public:
	class value{
	protected:
		bool i_, i__;
		value(){};
		HKEY _R;
		std::string const _k;
		std::string _v;
		std::string& GetValue(){
			unsigned char str[1024];
			DWORD sz=sizeof(str);
			REG_TRACE(printf("REG_TRACE:\treading value for key \"%s\" from registry ... ",_k.c_str()));
			if(ERROR_SUCCESS==RegQueryValueEx(_R,_k.c_str(),0,0,(unsigned char *)str,&sz)){
				REG_TRACE(printf("OK (read data size is %i bytes)\n",sz));
				_v=std::string((char*)str,sz);
				i_=true;
				i__=false;
			}else{
				REG_TRACE(printf("Failed!\n"));
			}
			return _v;
		}
		void SetValue(){
			REG_TRACE(printf("REG_TRACE:\twriting current value of key \"%s\" (data size is %i bytes) to registry ... ",_k.c_str(), _v.length()));
			if(ERROR_SUCCESS==RegSetValueEx(_R,_k.c_str(),0,REG_SZ,(const unsigned char *)_v.c_str(),_v.length())){
				REG_TRACE(printf("OK\n"));
				i__=false;
				i_=true;
			}else{
				REG_TRACE(printf("Failed!\n"));
			}
		}
		inline void __need_data(){if(!i_){GetValue();}}
	public:
		void refresh(){ GetValue(); }
		void flush(){ if(i__) SetValue(); }
		value(HKEY k, const char * val):_k(val),i_(false),i__(false),_R(k), _v("\0\0\0",sizeof("\0\0\0")){
			REG_TRACE(printf("REG_TRACE:\tconstructed object registry::value with name \"%s\"\n",_k.c_str()));
		}
		~value(){
			flush();
			REG_TRACE(printf("REG_TRACE:\tdestructed object registry::value with name \"%s\"\n",_k.c_str()));
		}

#define GET_SET(t) \
		operator t(){\
			__need_data();\
			if(_v.length()<sizeof(t)) { \
				REG_TRACE(printf("REG_TRACE:\tError: size of the read data (%i bytes) doesn't match requested size (%i bytes). Will return NULL!\n",_v.length(),sizeof(t))); \
				return(t)NULL;\
			}\
			return *((t*)(_v.c_str()));\
		} \
		/*if(_v.length()<sizeof(t)) _v.resize(sizeof(t)); */\
		const value & operator=(t v){ \
			_v=std::string((char*)&v, sizeof(t)); i_=true; i__=true; return *this; \
		}

		const value & operator=(const char * s){ _v=s; i_=true; i__=true; return *this;}
		const value & operator=(const unsigned char * s){ _v=(const char *)s; i_=true; i__=true; return *this;}
		//value & operator=(char * s){ _v=s; i_=true; i__=true; return *this;}
		const value & operator=(const value &v){_v=(std::string&)v; i_=true; i__=true; return *this;}
		const value & operator=(const std::string& s){_v=s; i_=true; i__=true; return *this;}
		//operator=(pair<const void*, int> p){_v=std::string((char*)p.first, p.second); i_=true;}

		GET_SET(const char);
		GET_SET(const short);
		GET_SET(const int);
		GET_SET(const long);
		GET_SET(const unsigned char);
		GET_SET(const unsigned short);
		GET_SET(const unsigned int);
		GET_SET(const unsigned long);

		operator const unsigned char* (){__need_data(); return (const unsigned char*)(_v.c_str());}
		operator const char*(){__need_data(); return _v.c_str();}
		operator const std::string&(){__need_data(); return _v;}
		operator bool(){__need_data(); return i_; }
		//operator pair<const void*, int>(){__need_data(); return make_pair((const void*)_v.c_str(), _v.length()); }

		template <class T>
		void set_struct(T str){
			_v=std::string((char*)&str, sizeof(T));
			 i_=true;
			 i__=true;
		};
		template <class T>
		bool get_struct(T& str){
			__need_data();
			if(_v.length()!=sizeof(T)){
				REG_TRACE(printf("REG_TRACE:\tError: size of the read data (%i bytes) doesn't match requested size (%i bytes). Will return null struct!\n",_v.length(),sizeof(T)));
				//memset((void*)&str,0,sizeof(T));  <- use this if the next line fails because there is no default constructor for T.
				str=T();
				return false;
			}else{
				memcpy((void*)&str,_v.c_str(), sizeof(T));
				return true;
			}
		};
	};

	typedef value iterator;

	iterator operator[](const char* val) const { return value(_R, val); }
	iterator operator[](const std::string & val)const{ return value(_R, val.c_str()); }
	operator bool() const{return _R!=0; }
	registry(const char*k,   Key key=hkey_local_machine):_R(0){ create_key(k, key); }
	registry(const std::string & k,Key key=hkey_local_machine):_R(0){ create_key(k.c_str(), key); }
	~registry(){
		REG_TRACE(printf("REG_TRACE:\tdestructing registry\n"));
		RegCloseKey(_R);
	}
};
#endif

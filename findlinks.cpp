#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

#include <time.h>
#ifdef _WIN32
#include <sys/utime.h>
#else
#include <sys/time.h>
#endif


#include <err/posix.h>
#include <fcntl.h>
#include <stringutils.h>
#include <utfcvutils.h>
#include <args.h>
#include <FileFunctions.h>  // GetFileInfo, AT_ISDIRECTORY
#include <cpputils/fhandle.h>
#include <cpputils/mmem.h>

// todo: skip <script> tags
// todo: skip PNG, JPG, GIF
// todo: add meta refresh content url

// parse html attribute value
template<typename P>
std::string getstring(P first, P last)
{
    P p= first;
    char quotechar=0;
    while (p<last && isspace(*p))
        p++;
    if (p==last || *p!='=')
        return "";
    p++;
    while (p<last && isspace(*p))
        p++;
    if (p==last)
        return "";

    if (*p=='"' || *p=='\'')
        quotechar= *p++;
    std::string str;
    P stringstart= p;
    while (p<last && *p!=quotechar && *p!='\n' && *p!='\r' && (quotechar || (*p!='>' && !isspace(*p))))
        p++;
    if (*p=='\n' || *p=='\r')
        return "";
    if (quotechar && *p!=quotechar)
        return "";

    return std::string(&*stringstart, p-stringstart);
}

std::vector<std::string> tld7= { ".museum", ".travel" };
std::vector<std::string> tld6= { ".onion"  };
std::vector<std::string> tld5= { ".aero", ".asia", ".coop", ".info", ".jobs", ".mobi", ".name" };
std::vector<std::string> tld4= { ".biz", ".cat", ".com", ".edu", ".gov", ".int", ".mil", ".net", ".org", ".pro", ".tel", ".xxx" };
std::vector<std::string> tld3= { ".ac", ".ad", ".ae", ".af", ".ag", ".ai", ".al", ".am", ".an", ".ao", ".aq", ".ar", ".as", ".at", ".au", ".aw", ".ax", ".az", ".ba", ".bb", ".bd", ".be", ".bf", ".bg", ".bh", ".bi", ".bj", ".bm", ".bn", ".bo", ".br", ".bs", ".bt", ".bv", ".bw", ".by", ".bz", ".ca", ".cc", ".cd", ".cf", ".cg", ".ch", ".ci", ".ck", ".cl", ".cm", ".cn", ".co", ".cr", ".cs", ".cu", ".cv", ".cx", ".cy", ".cz", ".de", ".dj", ".dk", ".dm", ".do", ".dz", ".ec", ".ee", ".eg", ".er", ".es", ".et", ".eu", ".fi", ".fj", ".fk", ".fm", ".fo", ".fr", ".ga", ".gb", ".gd", ".ge", ".gf", ".gg", ".gh", ".gi", ".gl", ".gm", ".gn", ".gp", ".gq", ".gr", ".gs", ".gt", ".gu", ".gw", ".gy", ".hk", ".hm", ".hn", ".hr", ".ht", ".hu", ".id", ".ie", ".il", ".im", ".in", ".io", ".iq", ".ir", ".is", ".it", ".je", ".jm", ".jo", ".jp", ".ke", ".kg", ".kh", ".ki", ".km", ".kn", ".kp", ".kr", ".kw", ".ky", ".kz", ".la", ".lb", ".lc", ".li", ".lk", ".lr", ".ls", ".lt", ".lu", ".lv", ".ly", ".ma", ".mc", ".md", ".me", ".mg", ".mh", ".mk", ".ml", ".mm", ".mn", ".mo", ".mp", ".mq", ".mr", ".ms", ".mt", ".mu", ".mv", ".mw", ".mx", ".my", ".mz", ".na", ".nc", ".ne", ".nf", ".ng", ".ni", ".nl", ".no", ".np", ".nr", ".nu", ".nz", ".om", ".pa", ".pe", ".pf", ".pg", ".ph", ".pk", ".pl", ".pm", ".pn", ".pr", ".ps", ".pt", ".pw", ".py", ".qa", ".re", ".ro", ".rs", ".ru", ".rw", ".sa", ".sb", ".sc", ".sd", ".se", ".sg", ".sh", ".si", ".sj", ".sk", ".sl", ".sm", ".sn", ".so", ".sr", ".st", ".su", ".sv", ".sy", ".sz", ".tc", ".td", ".tf", ".tg", ".th", ".tj", ".tk", ".tl", ".tm", ".tn", ".to", ".tp", ".tr", ".tt", ".tv", ".tw", ".tz", ".ua", ".ug", ".uk", ".us", ".uy", ".uz", ".va", ".vc", ".ve", ".vg", ".vi", ".vn", ".vu", ".wf", ".ws", ".ye", ".yt", ".za", ".zm", ".zw" };

bool ishostpath(const std::string& base)
{
    size_t ix= base.find_last_of('.');
    if (ix==base.npos)
        return false;
    std::string tld= base.substr(ix);
    switch(base.size()-ix) {
        case 3: return std::binary_search(tld3.begin(), tld3.end(), tld);
        case 4: return std::binary_search(tld4.begin(), tld4.end(), tld);
        case 5: return std::binary_search(tld5.begin(), tld5.end(), tld);
        case 6: return std::binary_search(tld6.begin(), tld6.end(), tld);
        case 7: return std::binary_search(tld7.begin(), tld7.end(), tld);
    }
    return false;
}
std::string gethostpath(const std::string& filename)
{
    std::string str= filename;
    while (!str.empty() && !ishostpath(str))
    {
        size_t ix= str.find_last_of('/');
        if (ix==str.npos)
            break;
        str.resize(ix);
    }
    return str;
}
std::string makepath(const std::string& filename, const std::string& path)
{
    if (path.find("://")!=path.npos)
        return path;
    if (path.find("javascript:")==0)
        return path;
    if (path.find("mailto:")==0)
        return path;
    if (!path.empty() && path[0]=='/')
        return gethostpath(filename)+path;
    return filename.substr(0, filename.find_last_of('/'))+"/"+path;
}
bool caseinsensitive(char lhs, char rhs) {
  return tolower(lhs)==tolower(rhs);
}
template<typename P>
void findstrings(const std::string& filename, P first, P last, const std::string& token)
{
    P p= first;
    while (p<last) {
        p= std::search(p, last, token.begin(), token.end(), &caseinsensitive);
        if (p==last)
            break;
        std::string imgpath= getstring(p+token.size(), last);
        if (!imgpath.empty())
            printf("%s\n", makepath(filename, imgpath).c_str());
        p++;
    }
}
bool istokenchar(char c)
{
    return isalnum(c);
}
template<typename P>
P nexthtmltoken(P first, P last, bool &isend, std::string& token)
{
    P p= std::find(first, last, '<');
    if (p==last) return last;
    p++; // skip '>'
    while (p<last && isspace(*p))
        p++;
    if (p==last) return last;

    if (*p=='/') {
        isend = true;
        p++; // skip '/'
        while (p<last && isspace(*p))
            p++;
        if (p==last) return last;
    }
    else {
        isend = false;
    }
    P tokenstart= p;
    while (p<last && istokenchar(*p))
        p++;
    if (p==last) return last;

    token= std::string(&*tokenstart, p-tokenstart);
    return p;
}
template<typename P, typename F>
P enumattrs(P first, P last, F f)
{
    P p= first;
    while (p<last) {
        while (p<last && isspace(*p))
            p++;
        if (p==last) return p;
        if (*p=='/' || *p=='>') {
            // expect />
            return p;
        }
        P keystart= p;
        while (p<last && *p!='>' && *p!='=' && !isspace(*p))
            p++;
        if (p==last || *p=='>') return p;

        std::string key(&*keystart, p-keystart);

        while (p<last && isspace(*p))
            p++;
        if (p==last || *p!='=')
            return p;
        p++; // skip '='
        while (p<last && isspace(*p))
            p++;
        if (p==last) return p;

        char quotechar=0;
        if (*p=='"' || *p=='\'')
            quotechar= *p++;
        std::string str;
        P stringstart= p;
        while (p<last && *p!=quotechar && (quotechar || !(*p=='>' || isspace(*p))))
            p++;

        std::string val(&*stringstart, p-stringstart);

        f(key, val);

        if (p<last && quotechar && *p==quotechar)
            p++;
    }
    return p;
}

template<typename P>
void findforms(const std::string& filename, P first, P last)
{
    P p= first;
    while (p<last) {
        std::string token;
        bool isend;
        p= nexthtmltoken(p, last, isend, token);
        if (p==last)
            break;
        if (stringicompare(token, std::string("form"))==0) {
            std::string method="GET", action="";
            p= enumattrs(p, last, [&method, &action](const std::string& key, const std::string& value)
            {
                if (stringicompare(key, std::string("method"))==0)
                    method= value;
                else if (stringicompare(key, std::string("action"))==0)
                    action= value;
            });
            printf("%s %s\n", method.c_str(), makepath(filename,action).c_str());
        }
        else if (stringicompare(token, std::string("input"))==0) {
            std::string name, val;
            p= enumattrs(p, last, [&name, &val](const std::string& key, const std::string& value)
            {
                if (stringicompare(key, std::string("name"))==0)
                    name= value;
                if (stringicompare(key, std::string("value"))==0)
                    val= value;
            });
            printf("\t%s = %s\n", name.c_str(), val.c_str());
        }
        else if (stringicompare(token, std::string("textarea"))==0) {
            std::string name, val;
            p= enumattrs(p, last, [&name](const std::string& key, const std::string& value)
            {
                if (stringicompare(key, std::string("name"))==0)
                    name= value;
            });
            P starttext= p;
            p= std::find(p, last, '<');
            val= std::string(&*starttext, p-starttext);
            printf("\t%s = %s\n", name.c_str(), val.c_str());
        }
        else if (stringicompare(token, std::string("select"))==0) {
            std::string name;
            p= enumattrs(p, last, [&name](const std::string& key, const std::string& value)
            {
                if (stringicompare(key, std::string("name"))==0)
                    name= value;
            });
            printf("\t%s\n", name.c_str());
        }
        else if (stringicompare(token, std::string("option"))==0) {
            std::string val;
            p= enumattrs(p, last, [&val](const std::string& key, const std::string& value)
            {
                if (stringicompare(key, std::string("value"))==0)
                    val= value;
            });
            printf("\t\t%s\n", val.c_str());
        }
    }
}
template<typename P>
void searchrange(const std::string& filename, P first, P last)
{
    findstrings(filename, first, last, "src");          // <img>, <javascript>, <embed>
    findstrings(filename, first, last, "href");         // <a>, <base>
    findstrings(filename, first, last, "action");       // <form>
    findstrings(filename, first, last, "codebase");     // <object>
    findstrings(filename, first, last, "archive");      // <applet>
    findstrings(filename, first, last, "pluginspage");  // <embed>
    findstrings(filename, first, last, "document.location");  // <javascript>

    findforms(filename, first, last);
}


template<typename V>
void read_stdin(V& data)
{
    while (1)
    {
        data.resize(data.size()+65536);
        int n= read(0, &data[data.size()-65536], 65536*sizeof(typename V::value_type));
        if (n==-1) {
            data.resize(data.size()-65536);
            break;
        }
        data.resize(data.size()-65536+n/sizeof(typename V::value_type));
        if (n==0)
            break;
    }
}
template<typename P>
bool isutf16(P first, P last)
{
    return (last-first)>4 && *(uint16_t*)first==0xfeff;
}
void processstdin()
{
    std::vector<char> data;

    read_stdin(data);

    try {
        if (isutf16(&data[0], &data[0]+data.size())) {
            std::vector<utf16char_t> u16((utf16char_t*)&data[0], (utf16char_t*)(&data[0]+(data.size()&~1)));
            u16.push_back(0);
            data.resize(utf16toutf8bytesneeded(&u16[0]));
            utf16toutf8(&u16[0], (utf8char_t*)&data[0]);
        }
        searchrange("[stdin]", data.begin(), data.end());
    }
    catch(...)
    {
        printf("EXCEPTION processing [stdin]\n");
    }
}
void processfile(const std::string& filename)
{
    try {
        filehandle f = open(filename.c_str(), O_RDONLY);
        mappedmem m(f, 0, f.size(), PROT_READ);

        if (isutf16(m.begin(), m.end())) {
            uint8_t *p= m.begin();
            uint8_t *pend= p+ (m.size()&~1);
            std::vector<utf16char_t> u16((utf16char_t*)p, (utf16char_t*)pend);
            u16.push_back(0);
            std::vector<char> data(utf16toutf8bytesneeded(&u16[0]));
            utf16toutf8(&u16[0], (utf8char_t*)&data[0]);

            searchrange(filename, data.begin(), data.end());
        }
        else {
            searchrange(filename, (char*)m.begin(), (char*)m.end());
        }
    }
    catch(...)
    {
        printf("EXCEPTION processing %s\n", filename.c_str());
    }
}
template<typename ACTION>
void processarg(const std::string& arg, ACTION act)
{
/*    if (arg[0]=='@') {
        // if name starts with '@' -> read list from file
        MmapReader rl(arg.substr(1), MmapReader::readonly);

        rl.line_enumerator([act](const char *first, const char*last)->bool {
            processarg(std::string(first, last-first), act);
            return true;
        });
    }
    else */
    switch (GetFileInfo(arg)) {
        case AT_ISDIRECTORY:
            // if <arg> is a directory -> add all files in that directory
            dir_iterator(arg,
                [act](const std::string& srcpath) {
                    act(srcpath);
                },
                [](const std::string& dirname)->bool { return true; }
            );
            break;
        case AT_ISFILE:
            {
            act(arg);
            }
            break;
        default:
            printf("WARNING: %s does not exist\n", arg.c_str());
    }
}

template<typename S>
bool extension_in(const typename S::value_type& item, const S& s)
{
    size_t ldot= item.find_last_of('.');
    if (ldot==item.npos)
        return false;
    if (s.find(item.substr(ldot))!=s.end())
        return true;
    if (ldot+1==item.size())
        return false;
    if (s.find(item.substr(ldot+1))!=s.end())
        return true;
    return false;
}

struct insensitiveless {
bool operator()(const std::string& L,const std::string& R) const {  return stringicompare(L,R)<0; }
};

int main(int argc,char**argv)
{
    std::vector<std::string> files;
    std::set<std::string, insensitiveless> filtertypes;
    try {
        bool nomoreoptions= false;
        bool dostdin= true;
    for (int i=1 ; i<argc ; i++)
    {
        if (!nomoreoptions && argv[i][0]=='-') switch(argv[i][1])
        {
            case 'i': filtertypes.insert(getstrarg(argv, i, argc)); break;
            case '-': nomoreoptions= true; break;
            case 0:  // '-'
                  dostdin= true;
        }
        else {
            dostdin= false;
            processarg(argv[i], [&files,&filtertypes](const std::string& filename)
                {
                    if (!extension_in(filename, filtertypes))
                        files.push_back(filename);
                }
            );
        }
    }
    if (files.empty() && dostdin)
        processstdin();

    std::for_each(files.begin(), files.end(), processfile);
    }
    catch(...)
    {
        printf("EXCEPTION\n");
    }
    return 0;
}

